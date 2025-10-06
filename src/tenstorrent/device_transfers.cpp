#include "device_transfers.hpp"
#include "kernel_launcher.hpp"
#include "lduAddressing.H"
#include "messageStream.H"
#include "lduMatrix.H"

#include "scalarField.H"
#include "tt-metalium/host_api.hpp"
#include "tt-metalium/tt_metal_profiler.hpp"
#include "ttLduData.hpp"
#include <cstddef>
#include <tuple>

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

void copy_scalarField_from_device(
    KernelLauncher& kernelLauncher,
    std::variant<std::reference_wrapper<tt::tt_metal::Buffer>, std::shared_ptr<tt::tt_metal::Buffer>> buffer,
    Foam::scalarField* field,
    uint32_t buf_offset
) {
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
        buffer,
        data,
        {buf_offset, padded_bytes},
        true
    );

    if (bytes != padded_bytes) {
        std::memcpy(field->data(), data, bytes);
        delete[] data;
    }

    tt::tt_metal::detail::DumpDeviceProfileResults(device);
}

void copy_scalarField_from_device(KernelLauncher& kernelLauncher, ReusableTtBuffer& buffer, Foam::scalarField* field, uint32_t buf_offset) {
    copy_scalarField_from_device(kernelLauncher, buffer.buffer, field, buf_offset);
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

    if (used_iface_count) {

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

    } else {
        return {ReusableTtBuffer::unusedPlaceholder(), 0};
    }
}

uint32_t offset_into_tiled_mat(uint32_t row, uint32_t col, uint32_t line_lenght) {
    auto tile_row = row / tt::constants::TILE_HEIGHT;
    auto tile_col = col / tt::constants::TILE_WIDTH;
    auto in_tile_row = row % tt::constants::TILE_HEIGHT;
    auto in_tile_col = col % tt::constants::TILE_WIDTH;

    auto tile_row_start = tile_row * tt::constants::TILE_HEIGHT * line_lenght;
    auto tile_start = tile_row_start + tile_col * (tt::constants::TILE_HEIGHT * tt::constants::TILE_WIDTH);

    return tile_start + in_tile_row * tt::constants::TILE_WIDTH + in_tile_col;
}

void copy_ldu_to_dense(KernelLauncher& k, tt_ldu_meta& tt_meta, const Foam::lduMatrix* lduMat) {

    auto cells = lduMat->lduAddr().size();
    auto aligned_cells = round_up(cells, tt::constants::TILE_WIDTH);
    auto page_size = tt::constants::TILE_HEIGHT * tt::constants::TILE_WIDTH*sizeof(float);

    auto buf_size = aligned_cells*aligned_cells;

    float* dense = new float[buf_size];

    tt_meta.cell_count = cells;

    if (!tt_meta.d_dense_) {
        tt_meta.d_dense_ = tt::tt_metal::CreateBuffer({
            .device = k.device_,
            .size = buf_size*sizeof(float),
            .page_size = page_size,
            .buffer_type = tt::tt_metal::BufferType::DRAM
        });
    }

    auto hasDiag = lduMat->hasDiag();
    auto diag = hasDiag? lduMat->diag().cdata() : nullptr;

    for (int i = 0; i < cells; ++i) {
        dense[offset_into_tiled_mat(i, i, aligned_cells)] = hasDiag? diag[i] : 0.0f;
    }
    auto lowerAddr = lduMat->lduAddr().lowerAddr();
    auto upperAddr = lduMat->lduAddr().upperAddr();
    auto sparse_vals = lowerAddr.size();
    auto hasLower = lduMat->hasLower();
    auto lower = hasLower? lduMat->lower().cdata() : nullptr;
    auto hasUpper = lduMat->hasUpper();
    auto upper = hasUpper? lduMat->upper().cdata() : nullptr;

    for (int i= 0; i < sparse_vals; ++i) {
        auto lowAddr = lowerAddr[i];
        auto upAddr = upperAddr[i];
        float l_val = hasLower? lower[i] : 0.0f;
        float u_val = hasUpper? upper[i] : 0.0f;

        dense[offset_into_tiled_mat(upAddr, lowAddr, aligned_cells)] = l_val;
        dense[offset_into_tiled_mat(lowAddr, upAddr, aligned_cells)] = u_val;
    }

    tt::tt_metal::EnqueueWriteBuffer(
        k.device_->command_queue(0),
        tt_meta.d_dense_,
        dense,
        true
    );

    delete[] dense;

    tt_meta.dense_on_device_ = true;
}

std::tuple<bool, bool, bool> copy_ldu_from_dense(
    KernelLauncher& k,
    tt_ldu_meta& tt_meta,
    Foam::scalarField* diagField,
    Foam::scalarField* lowerField,
    Foam::scalarField* upperField,
    const Foam::lduAddressing& lduAddressing
) {

    if (!tt_meta.d_dense_ || !tt_meta.dense_on_device_) {
        throw new std::runtime_error("copy_ldu_from_dense: dense not on device");
    }

    auto cells = tt_meta.cell_count;
    auto aligned_cells = round_up(cells, tt::constants::TILE_WIDTH);

    auto buf_size = aligned_cells*aligned_cells;

    float* dense = new float[buf_size];

    tt::tt_metal::EnqueueReadBuffer(
        k.device_->command_queue(0),
        tt_meta.d_dense_,
        dense,
        true
    );

    bool diagNonZero = false;
    if (diagField) {
        float* diag = diagField->data();
        for (uint32_t i = 0; i < cells; ++i) {
            auto val = dense[offset_into_tiled_mat(i, i, aligned_cells)];
            diagNonZero |= val != 0.0f;
            diag[i] = val;
        }
    }

    auto lowerAddr = lduAddressing.lowerAddr();
    auto upperAddr = lduAddressing.upperAddr();
    auto sparse_vals = lowerAddr.size();
    
    auto lower = lowerField? lowerField->data() : nullptr;
    auto upper = upperField? upperField->data() : nullptr;

    bool lowerNonZero = false;
    bool upperNonZero = false;

    if (lowerField || upperField) {
        for (int32_t i= 0; i < sparse_vals; ++i) {
            auto lowAddr = lowerAddr[i];
            auto upAddr = upperAddr[i];
            if (lower) {
                auto val = dense[offset_into_tiled_mat(upAddr, lowAddr, aligned_cells)];
                lower[i] = val;
                lowerNonZero |= (lower[i] != 0.0f);
            }
            if (upper) {
                auto val = dense[offset_into_tiled_mat(lowAddr, upAddr, aligned_cells)];
                upper[i] = val;
                upperNonZero |= (upper[i] != 0.0f);
            }
        }
    }

    delete[] dense;

    return {diagNonZero, lowerNonZero, upperNonZero};
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

    // Foam::Info << "Copying addresses to device for " << reinterpret_cast<const void*>(lduMat) << Foam::endl;

    auto* device = k.device_;

    tt_meta.sparse_count = lduMat->lduAddr().lowerAddr().size();
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

    // Foam::Info << "  Total size " << total_size << ", triang_bytes " << triangBytes << ", interface_meta_bytes " << interfaceMetaBytes << Foam::endl;

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


void copy_ldu_contents_to_device(KernelLauncher& k,tt_ldu_meta& tt_meta, const Foam::lduMatrix* lduMat, bool reserve_all) {
    // Foam::Info << "Copying contents to device for " << reinterpret_cast<const void*>(lduMat) << Foam::endl;

    auto* device = k.device_;

    bool hasDiag = lduMat->hasDiag();
    tt_meta.diag_zero = !hasDiag;
    tt_meta.cell_count = lduMat->lduAddr().size();
    auto diagBytes = tt::round_up(sizeof(float)*tt_meta.cell_count, tt_block_size);
    size_t triang_bytes = tt::round_up(sizeof(float)*(lduMat->lduAddr().lowerAddr().size()), tt_block_size);
    size_t lower_bytes, upper_bytes;
    bool hasLower = lduMat->hasLower();
    bool hasUpper = lduMat->hasUpper();
    tt_meta.triang_zero = !(hasLower || hasUpper);
    if ((hasLower && !hasUpper) || (!hasLower && hasUpper) || (!hasLower && !hasUpper)) {
        tt_meta.lower_contains_also_upper = true; // do this irrespective of address layout. I.e. if this flag is set, upper data needs to be read from lower. Or, without reserve_all it can also be read from upper
    } else {
        tt_meta.lower_contains_also_upper = false;
    }

    if (hasLower || reserve_all) {
        lower_bytes = triang_bytes;
    } else {
        lower_bytes = 0;
    }
    if (hasUpper || reserve_all) {
        upper_bytes = triang_bytes;
    } else {
        upper_bytes = 0;
    }

    size_t total_size = diagBytes + lower_bytes + upper_bytes;

    if (!tt_meta.d_data_ || tt_meta.d_data_->size() < total_size) { // if we need more size for reservation, we may need to reallocate
        tt_meta.d_data_ = tt::tt_metal::CreateBuffer({
            .device = device,
            .size = total_size,
            .page_size = tt_block_size,
            .buffer_type = tt::tt_metal::BufferType::DRAM
        });
    }

    if (hasDiag) {
        tt::tt_metal::EnqueueWriteSubBuffer(
            device->command_queue(0),
            tt_meta.d_data_,
            lduMat->diag().cdata(),
            {0, diagBytes},
            false
        );
    }
    tt_meta.lower_contents_start_ = diagBytes/4; // we always reserve diag
    if (hasLower) {
        tt::tt_metal::EnqueueWriteSubBuffer(
            device->command_queue(0),
            tt_meta.d_data_,
            lduMat->lower().cdata(),
            {tt_meta.lower_contents_start_*4, lower_bytes},
            false
        );
    }
    if (hasUpper || reserve_all) {
        tt_meta.upper_contents_start_ = tt_meta.lower_contents_start_ + lower_bytes/4;
    } else {
        tt_meta.upper_contents_start_ = tt_meta.lower_contents_start_; // reuse the lower part
    }
    if (hasUpper) { // transfer
        uint32_t upper_write_offset = hasLower? tt_meta.upper_contents_start_*4 : tt_meta.lower_contents_start_*4;
        tt::tt_metal::EnqueueWriteSubBuffer(
            device->command_queue(0),
            tt_meta.d_data_,
            lduMat->upper().cdata(),
            {upper_write_offset, upper_bytes},
            false
        );
    }
    
    

    tt_meta.contents_on_device_ = true;
}