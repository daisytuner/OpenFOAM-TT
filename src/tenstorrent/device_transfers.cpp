#include "device_transfers.hpp"
#include "kernel_launcher.hpp"
#include "messageStream.H"
#include "LduMatrix.H"

ReusableTtBuffer& copy_scalarField_to_device(KernelLauncher& kernelLauncher, const Foam::scalarField& field) {
    auto* device = kernelLauncher.device_;

    size_t bytes = sizeof(float)*field.size();

    auto& buffer = kernelLauncher.allocateBuffer(bytes);

    tt::tt_metal::EnqueueWriteSubBuffer(
        device->command_queue(0),
        buffer.buffer,
        field.cdata(),
        {0, round_up(bytes, tt_block_size)},
        false
    );

    return buffer;
}

void copy_scalarField_from_device(KernelLauncher& kernelLauncher, ReusableTtBuffer& buffer, Foam::scalarField* field) {
    auto* device = kernelLauncher.device_;

    size_t bytes = sizeof(float)*field->size();
    size_t padded_bytes = round_up(bytes, tt_block_size);

    float* data = nullptr;
    if (bytes == padded_bytes) {
        data = field->data();
    } else {
        data = new float[padded_bytes/sizeof(float)];
    }

    tt::tt_metal::EnqueueReadSubBuffer(
        device->command_queue(0),
        buffer.buffer,
        data,
        {0, padded_bytes},
        true
    );

    if (bytes != padded_bytes) {
        std::memcpy(field->data(), data, bytes);
        delete[] data;
    }
}

/**
 * For each set Interface :
 *  0: interface idx
 *  1: number of coeffs
 *  2:    coeffs
 *        ...
 *  n: interface idx
 * This allows skipping interfaces that are not set
 */
std::pair<ReusableTtBuffer&, int> copy_interfaceCoeffs_to_device(
    KernelLauncher& kernelLauncher,
    const Foam::FieldField<Foam::Field, Foam::scalar>& interfaceCoeffs,
    const Foam::lduInterfaceFieldPtrsList& interfaces
) {

    auto* device = kernelLauncher.device_;

    size_t total_size = 0;
    auto iface_count = interfaces.size();
    auto used_iface_count = 0;

    for (int i = 0; i < iface_count; ++i) {
        if (interfaces.set(i)) {
            total_size += 4 + 4; // size
            auto size = interfaceCoeffs[i].size();
            total_size += size * sizeof(float);
            used_iface_count++;
        }
    }

    auto& buffer = kernelLauncher.allocateBuffer(total_size);

    std::vector<uint32_t> interface_data(total_size / sizeof(uint32_t));
    size_t idx = 0;
    for (int i = 0; i < iface_count; ++i) {
        if (interfaces.set(i)) {
            interface_data[idx++] = i;
            auto num_coeffs = interfaceCoeffs[i].size();
            interface_data[idx++] = num_coeffs;
        
            for (int j = 0; j < interfaceCoeffs[i].size(); ++j) {
                interface_data[idx++] = reinterpret_cast<const uint32_t&>(interfaceCoeffs[i][j]);
            }
        }
    }

    tt::tt_metal::EnqueueWriteSubBuffer(
        device->command_queue(0),
        buffer.buffer,
        interface_data.data(),
        {0, total_size},
        false
    );

    return {buffer, used_iface_count};


}

/**
 * iface addr:
 *  0: iface 0 offset (= 2)
 *  1: iface 1 offset (= 8)

 *  2: iface 0, num coeffs
 *  3:   - coeffs
 *      ...
 *  8: iface 1, num coeffs
 *  9:   - coeffs
 *      ...
 *
 * counts could be gotten from addr instead, as well but when message-passing them through NOC we may also have additional alignment issues
 */
void copy_ldu_addrs_to_device(KernelLauncher& k,tt_ldu_meta& tt_meta, const Foam::lduMatrix* lduMat) {

    Foam::Info << "Copying addresses to device for " << reinterpret_cast<const void*>(lduMat) << Foam::endl;

    auto* device = k.device_;

    tt_meta.sparse_count = lduMat->lower().size();
    auto triangBytes = tt::round_up(sizeof(float)*tt_meta.sparse_count, tt_block_size);
    size_t interfaceBytesSum = 0;
    auto iface_count = lduMat->mesh().interfaces().size();
    std::vector<uint32_t> interface_start_offsets { static_cast<uint32_t>(iface_count) };
    for (int i = 0; i < iface_count; ++i) {
        interfaceBytesSum += 4 + 4;
        if (lduMat->mesh().interfaces().set(i)) {
            auto size = lduMat->lduAddr().patchAddr(i).size();
            interfaceBytesSum += size * sizeof(uint32_t);
            interface_start_offsets.push_back(interface_start_offsets.back() + 1 + size);
        } else {
            interface_start_offsets.push_back(interface_start_offsets.back() + 1);
        }
    }
    auto interfaceMetaBytes = tt::round_up(interfaceBytesSum, tt_block_size);
    size_t total_size = triangBytes + triangBytes + interfaceMetaBytes;

    if (!tt_meta.d_addrs_) {
        tt_meta.d_addrs_ = tt::tt_metal::CreateBuffer({
            .device = device,
            .size = total_size,
            .page_size = tt_block_size,
            .buffer_type = tt::tt_metal::BufferType::DRAM
        });
    } else {
        if (tt_meta.d_addrs_->size() != total_size) {
            Foam::Warning << "Reallocating address buffer from " << tt_meta.d_addrs_->size() << " to " << total_size << Foam::endl;
            throw new std::runtime_error("Reallocating address buffer, not implemented");
        }
    }

    Foam::Info << "  Total size " << total_size << ", triang_bytes " << triangBytes << ", interface_meta_bytes " << interfaceMetaBytes << Foam::endl;

    tt::tt_metal::EnqueueWriteSubBuffer(
        device->command_queue(0),
        tt_meta.d_addrs_,
        lduMat->lduAddr().lowerAddr().cdata(),
        {0, triangBytes},
        false
    );
    tt_meta.upper_addrs_start_ = triangBytes/4;
    tt::tt_metal::EnqueueWriteSubBuffer(
        device->command_queue(0),
        tt_meta.d_addrs_,
        lduMat->lduAddr().upperAddr().cdata(),
        {triangBytes, triangBytes},
        false
    );
    tt_meta.iface_map_start_ = (triangBytes + triangBytes)/4;
    std::vector<uint32_t> interface_addr_tmps(interfaceMetaBytes / sizeof(uint32_t));
    uint32_t idx = 0;
    for (int i = 0; i < iface_count; ++i) {
        interface_addr_tmps[idx++] = interface_start_offsets[i];
    }
    for (int i = 0; i < iface_count; ++i) {
        if (lduMat->mesh().interfaces().set(i)) {
            auto& patch = lduMat->lduAddr().patchAddr(i);
            auto count = patch.size();
            auto* vec = patch.begin();
            interface_addr_tmps[idx++] = count;
            auto end = idx + count;
            while (idx < end) {
                interface_addr_tmps[idx++] = *vec++;
            }
        } else {
            interface_addr_tmps[idx++] = 0;
        }
    }

    tt::tt_metal::EnqueueWriteSubBuffer(
        device->command_queue(0),
        tt_meta.d_addrs_,
        interface_addr_tmps,
        {tt_meta.iface_map_start_*4, interfaceMetaBytes},
        false
    );
    

    tt_meta.addrs_on_device_ = true;
}


void copy_ldu_contents_to_device(KernelLauncher& k,tt_ldu_meta& tt_meta, const Foam::lduMatrix* lduMat) {
    Foam::Info << "Copying contents to device for " << reinterpret_cast<const void*>(lduMat) << Foam::endl;

    auto* device = k.device_;

    tt_meta.cell_count = lduMat->diag().size();
    auto diagBytes = tt::round_up(sizeof(float)*tt_meta.cell_count, tt_block_size);
    auto triangBytes = tt::round_up(sizeof(float)*(lduMat->lower().size()), tt_block_size);

    size_t total_size = diagBytes + triangBytes + triangBytes;

    if (!tt_meta.d_data_) {
        tt_meta.d_data_ = tt::tt_metal::CreateBuffer({
            .device = device,
            .size = total_size,
            .page_size = tt_block_size,
            .buffer_type = tt::tt_metal::BufferType::DRAM
        });
    }

    tt::tt_metal::EnqueueWriteSubBuffer(
        device->command_queue(0),
        tt_meta.d_data_,
        lduMat->diag().cdata(),
        {0, diagBytes},
        false
    );
    tt_meta.lower_contents_start_ = diagBytes/4;
    tt::tt_metal::EnqueueWriteSubBuffer(
        device->command_queue(0),
        tt_meta.d_data_,
        lduMat->lower().cdata(),
        {tt_meta.lower_contents_start_*4, triangBytes},
        false
    );
    tt_meta.upper_contents_start_ = diagBytes/4 + triangBytes/4;
    tt::tt_metal::EnqueueWriteSubBuffer(
        device->command_queue(0),
        tt_meta.d_data_,
        lduMat->upper().cdata(),
        {tt_meta.upper_contents_start_*4, triangBytes},
        false
    );
    
    

    tt_meta.contents_on_device_ = true;
}