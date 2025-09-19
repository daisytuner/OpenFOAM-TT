#include "kernel_launcher.hpp"
#include "lduInterfacePtrsList.H"
#include "lduMatrix.H"
#include "tt-metalium/math.hpp"
#include "ttLduData.hpp"
#include <cstdint>
#include <tt-metalium/host_api.hpp>

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
void Foam::lduMatrix::copy_addrs_to_device(tt_ldu_meta& tt_meta) const {

    Foam::Info << "Copying addresses to device for " << reinterpret_cast<const void*>(this) << Foam::endl;

    auto& k = require_kernel_launcher();
    auto* device = k.device_;

    tt_meta.sparse_count = this->lower().size();
    auto triangBytes = tt::round_up(sizeof(float)*tt_meta.sparse_count, tt_block_size);
    size_t interfaceBytesSum = 0;
    auto iface_count = lduMesh_.interfaces().size();
    std::vector<uint32_t> interface_start_offsets { static_cast<uint32_t>(iface_count) };
    for (int i = 0; i < iface_count; ++i) {
        interfaceBytesSum += 4 + 4;
        if (mesh().interfaces().set(i)) {
            auto size = lduAddr().patchAddr(i).size();
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
        this->lduAddr().lowerAddr().cdata(),
        {0, triangBytes},
        false
    );
    tt_meta.upper_addrs_start_ = triangBytes/4;
    tt::tt_metal::EnqueueWriteSubBuffer(
        device->command_queue(0),
        tt_meta.d_addrs_,
        this->lduAddr().upperAddr().cdata(),
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
        if (mesh().interfaces().set(i)) {
            auto& patch = lduAddr().patchAddr(i);
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


void Foam::lduMatrix::copy_contents_to_device(tt_ldu_meta& tt_meta) const {
    Foam::Info << "Copying contents to device for " << reinterpret_cast<const void*>(this) << Foam::endl;

    auto& k = require_kernel_launcher();
    auto* device = k.device_;

    tt_meta.cell_count = this->diag().size();
    auto diagBytes = tt::round_up(sizeof(float)*tt_meta.cell_count, tt_block_size);
    auto triangBytes = tt::round_up(sizeof(float)*(lower().size()), tt_block_size);

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
        this->diag().cdata(),
        {0, diagBytes},
        false
    );
    tt_meta.lower_contents_start_ = diagBytes/4;
    tt::tt_metal::EnqueueWriteSubBuffer(
        device->command_queue(0),
        tt_meta.d_data_,
        this->lower().cdata(),
        {tt_meta.lower_contents_start_*4, triangBytes},
        false
    );
    tt_meta.upper_contents_start_ = diagBytes/4 + triangBytes/4;
    tt::tt_metal::EnqueueWriteSubBuffer(
        device->command_queue(0),
        tt_meta.d_data_,
        this->upper().cdata(),
        {tt_meta.upper_contents_start_*4, triangBytes},
        false
    );
    
    

    tt_meta.contents_on_device_ = true;
}