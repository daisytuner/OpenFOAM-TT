#include "device_transfers.hpp"
#include "ReusableTtBuffer.hpp"
#include "buffer_pool.hpp"
#include "kernel_launcher.hpp"
#include "lduAddressing.H"
#include "ldu_meta_cache.hpp"
#include "messageStream.H"
#include "lduMatrix.H"

#include "scalarField.H"
#include "tt-metalium/constants.hpp"
#include "tt-metalium/device.hpp"
#include "tt-metalium/host_api.hpp"
#include "tt-metalium/math.hpp"
#include "tt-metalium/tt_backend_api_types.hpp"
#include "tt-metalium/tt_metal_profiler.hpp"
#include "tt-metalium/util.hpp"
#include "ttLduData.hpp"
#include "tt_impls.hpp"
#include <cstddef>
#include <cstdint>
#include <limits.h>
#include <tuple>

namespace tt::daisy::foam {

#define TT_DEBUG 1

uint32_t offset_into_tiled_mat(uint32_t row, uint32_t col, uint32_t line_lenght) {
    auto tile_row = row / tt::constants::TILE_HEIGHT;
    auto tile_col = col / tt::constants::TILE_WIDTH;
    auto in_tile_row = row % tt::constants::TILE_HEIGHT;
    auto in_tile_col = col % tt::constants::TILE_WIDTH;

    auto tile_row_start = tile_row * tt::constants::TILE_HEIGHT * line_lenght;
    auto tile_start = tile_row_start + tile_col * (tt::constants::TILE_HEIGHT * tt::constants::TILE_WIDTH);

    auto face_row = in_tile_row / tt::constants::FACE_HEIGHT;
    auto face_col = in_tile_col / tt::constants::FACE_WIDTH;
    auto in_face_row = in_tile_row % tt::constants::FACE_HEIGHT;
    auto in_face_col = in_tile_col % tt::constants::FACE_WIDTH;
    auto face_idx = face_row * 2 + face_col;
    auto face_start = tile_start + face_idx * (tt::constants::FACE_HEIGHT * tt::constants::FACE_HEIGHT);

    return face_start + in_face_row * tt::constants::FACE_WIDTH + in_face_col;
}

ReusableTtBuffer& allocate_field_buffer_bare(BufferPool& bufferPool, uint32_t num_elements) {
    size_t bytes = sizeof(float)*num_elements;

    return bufferPool.allocateBuffer(bytes, tt_block_size);
}

ReusableTtBuffer& allocate_field_buffer_1tile(BufferPool& bufferPool, uint32_t num_elements) {
    size_t tiles = (num_elements + 31) / 32;
    size_t tileBytes = tt::tt_metal::detail::TileSize(tt::DataFormat::Float32);
    size_t bytes = tiles * tileBytes;

    return bufferPool.allocateBuffer(bytes, tileBytes);
}

ReusableTtBuffer& allocate_field_buffer(BufferPool& bufferPool, uint32_t num_elements) {
    
    #if TT_IMPL == TT_IMPL_LDU || TT_IMPL == TT_IMPL_ELLPACK
        return allocate_field_buffer_bare(bufferPool, num_elements);
    #elif TT_IMPL == TT_IMPL_DENSE
        return allocate_field_buffer_1tile(bufferPool, num_elements);
    #else
        #error unsupported TT IMPL TT_IMPL
    #endif
}

ReusableTtBuffer& copy_scalarField_to_device_bare(BufferPool& bufferPool, const Foam::scalarField& field) {
    auto* device = bufferPool.device_;

    size_t bytes = sizeof(float)*field.size();

    auto& buffer = bufferPool.allocateBuffer(bytes, tt_block_size);

    tt::tt_metal::EnqueueWriteSubBuffer(
        device->command_queue(0),
        buffer.buffer,
        field.cdata(),
        {0, tt::round_up(bytes, tt_block_size)},
        false
    );

    return buffer;
}

ReusableTtBuffer& copy_scalarField_to_device_as_dense_mat(BufferPool& bufferPool, const Foam::scalarField& field) {
    auto* device = bufferPool.device_;

    auto tile_size = tt::tt_metal::detail::TileSize(tt::DataFormat::Float32);
    auto tile_entries = 32*32;

    size_t tiles = (field.size() + 31) / 32;
    size_t bytes = tiles * tile_size;

    auto& buffer = bufferPool.allocateBuffer(bytes, tile_size);

    float* tilized_vector = new float[tile_entries];

    const float* field_ptr = field.cdata();

    for (uint32_t i = 0; i < tiles; i++) {

        int field_offset = i * 32;

        // std::memset(tilized_vector, 0, tile_size);

        size_t tmp_tile_in_total_buffer_offset = i * tile_entries;
        auto field_size = field.size();

        for (int element = field_offset; element < field_offset + 32 && element < field_size; ++element) {
            auto tilized_offset = offset_into_tiled_mat(element, 0, 32);
            tilized_vector[tilized_offset - tmp_tile_in_total_buffer_offset] = field_ptr[element];
        }

        tt::tt_metal::EnqueueWriteSubBuffer(
            device->command_queue(0),
            buffer.buffer,
            tilized_vector,
            {i * tile_size, tile_size},
            true
        );
    }

    delete[] tilized_vector;

    return buffer;
}

ReusableTtBuffer& copy_scalarField_to_device(BufferPool& bufferPool, const Foam::scalarField& field) {
    #if TT_IMPL == TT_IMPL_LDU || TT_IMPL == TT_IMPL_ELLPACK
        return copy_scalarField_to_device_bare(bufferPool, field);
    #elif TT_IMPL == TT_IMPL_DENSE
        return copy_scalarField_to_device_as_dense_mat(bufferPool, field);
    #else
        #error unsupported TT IMPL TT_IMPL
    #endif
}

void copy_scalarField_from_device_dense_mat(
    BufferPool& bufferPool,
    std::variant<std::reference_wrapper<tt::tt_metal::Buffer>, std::shared_ptr<tt::tt_metal::Buffer>> buffer,
    Foam::scalarField* field,
    uint32_t buf_offset
) {

    auto* device = bufferPool.device_;

    size_t tile_size = tt::tt_metal::detail::TileSize(tt::DataFormat::Float32);
    auto tile_entries = 32*32;

    size_t tiles = (field->size() + 31) / 32;

    float* tilized_vector = new float[tile_entries];

    for (uint32_t i = 0; i < tiles; i++) {

        int field_offset = i * 32;

        tt::tt_metal::EnqueueReadSubBuffer(
            device->command_queue(0),
            buffer,
            tilized_vector,
            {i * tile_size, tile_size},
            true
        );

        float* data = nullptr;
        data = field->data();

        size_t tmp_tile_in_total_buffer_offset = i * tile_entries;
        auto field_size = field->size();

        for (int element = field_offset; element < field_offset + 32 && element < field_size; ++element) {
            data[element] = tilized_vector[offset_into_tiled_mat(element, 0, 32) - tmp_tile_in_total_buffer_offset];
        }
    }

    delete[] tilized_vector;

    tt::tt_metal::detail::ReadDeviceProfilerResults(device);

}

void copy_scalarField_from_device_dense_mat(BufferPool& bufferPool, ReusableTtBuffer& buffer, Foam::scalarField* field, uint32_t buf_offset) {
    copy_scalarField_from_device_dense_mat(bufferPool, buffer.buffer, field, buf_offset);
}

void copy_scalarField_from_device_bare(
    BufferPool& bufferPool,
    std::variant<std::reference_wrapper<tt::tt_metal::Buffer>, std::shared_ptr<tt::tt_metal::Buffer>> buffer,
    Foam::scalarField* field,
    uint32_t buf_offset
) {
    auto* device = bufferPool.device_;

    size_t bytes = sizeof(float)*field->size();
    size_t padded_bytes = tt::round_up(bytes, tt_block_size);

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

    tt::tt_metal::detail::ReadDeviceProfilerResults(device);
}

void copy_scalarField_from_device_bare(BufferPool& bufferPool, ReusableTtBuffer& buffer, Foam::scalarField* field, uint32_t buf_offset) {
    copy_scalarField_from_device_bare(bufferPool, buffer.buffer, field, buf_offset);
}

void copy_scalarField_from_device(
    BufferPool& bufferPool,
    std::variant<std::reference_wrapper<tt::tt_metal::Buffer>, std::shared_ptr<tt::tt_metal::Buffer>> buffer,
    Foam::scalarField* field,
    uint32_t buf_offset
) {
    #if TT_IMPL == TT_IMPL_LDU || TT_IMPL == TT_IMPL_ELLPACK
        copy_scalarField_from_device_bare(bufferPool, buffer, field, buf_offset);
    #elif TT_IMPL == TT_IMPL_DENSE
        copy_scalarField_from_device_dense_mat(bufferPool, buffer, field, buf_offset);
    #else
        #error unsupported TT IMPL TT_IMPL
    #endif
}

void copy_scalarField_from_device(
    BufferPool& bufferPool,
    ReusableTtBuffer& buffer,
    Foam::scalarField* field,
    uint32_t buf_offset
) {
    copy_scalarField_from_device(bufferPool, buffer.buffer, field, buf_offset);
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
    BufferPool& bufferPool,
    const Foam::FieldField<Foam::Field, Foam::scalar>& interfaceCoeffs,
    const Foam::lduInterfaceFieldPtrsList& interfaces
) {

    auto* device = bufferPool.device_;

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

        auto& buffer = bufferPool.allocateBuffer(total_size, tt_block_size);

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

void copy_ldu_to_dense(tt::tt_metal::IDevice* device, tt_ldu_meta& tt_meta, const Foam::lduMatrix* lduMat) {

    auto cells = lduMat->lduAddr().size();
    auto aligned_cells = tt::round_up(cells, tt::constants::TILE_WIDTH);
    auto page_size = tt::constants::TILE_HEIGHT * tt::constants::TILE_WIDTH*sizeof(float);

    auto buf_size = aligned_cells*aligned_cells;

    float* dense = new float[buf_size];

    memset(dense, 0, buf_size*sizeof(float));

    tt_meta.cell_count = cells;

    if (!tt_meta.d_dense_) {
        tt_meta.d_dense_ = tt::tt_metal::CreateBuffer({
            .device = device,
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
        float u_val = hasUpper? upper[i] : 0.0f;
        float l_val = hasLower? lower[i] : u_val;

        dense[offset_into_tiled_mat(upAddr, lowAddr, aligned_cells)] = l_val;
        dense[offset_into_tiled_mat(lowAddr, upAddr, aligned_cells)] = u_val;
    }

    #if TT_DEBUG > 0
    printf("mat %ux%u:\n", aligned_cells, aligned_cells);
//    for (int i = 0; i < 32; ++i) {
//        for (int j = 0; j < 32; ++j) {
//            if (j == 16) {
//                printf("| ");
//            }
//            int face_begin = (i >= 16 ? (16*16*2) : 0) + (j >= 16 ? (16*16) : 0);
//            int in_face_x = j < 16 ? j : j-16;
//            int in_face_y = i < 16 ? i : i-16;
//            int idx = face_begin + in_face_y * 16 + in_face_x;
//            printf("%6.3f ", dense[idx]);
//        }
//        printf("\n");
//        if (i == 15) {
//            for (int j = 0; j < 32; ++j) {
//                printf("------ ");
//            }
//            printf("\n");
//        }
//    }

    #if TT_DEBUG > 1
    for (uint32_t i = 0; i < aligned_cells; ++i) {
        for (uint32_t j = 0; j < aligned_cells; ++j) {
            printf("%6.2f ", dense[i*aligned_cells + j]);
            if (j > 0 && (j+1) % 16 == 0) {
                if ((j+1) % 32 == 0) {
                    printf("|| ");
                } else {
                    printf("| ");
                }
            }
        }
        printf("\n");
        if (i > 0 && i % 16 == 0) {
            if (i % 32 == 0) {
                printf("========================================\n");
            } else {
                printf("----------------------------------------\n");
            }
        }
    }
    #endif
    #endif

    tt::tt_metal::EnqueueWriteBuffer(
        device->command_queue(0),
        tt_meta.d_dense_,
        dense,
        true
    );

    delete[] dense;

    tt_meta.dense_on_device_ = true;
}

std::tuple<bool, bool, bool> copy_ldu_from_dense(
    tt::tt_metal::IDevice* device,
    tt_ldu_meta& tt_meta,
    Foam::scalarField* diagField,
    Foam::scalarField* lowerField,
    Foam::scalarField* upperField,
    const Foam::lduAddressing& lduAddressing
) {

    if (!tt_meta.d_dense_ || !tt_meta.dense_on_device_) {
        throw std::runtime_error("copy_ldu_from_dense: dense not on device");
    }

    auto cells = tt_meta.cell_count;
    auto aligned_cells = tt::round_up(cells, tt::constants::TILE_WIDTH);

    auto buf_size = aligned_cells*aligned_cells;

    float* dense = new float[buf_size];

    tt::tt_metal::EnqueueReadBuffer(
        device->command_queue(0),
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

std::tuple<bool, bool, bool> copy_ldu_from_ellpack(
    tt::tt_metal::IDevice* device,
    tt_ldu_meta& tt_meta,
    Foam::scalarField* diagField,
    Foam::scalarField* lowerField,
    Foam::scalarField* upperField,
    const Foam::lduAddressing& lduAddressing
) {

    if (!tt_meta.d_ellpack_vals_ || !tt_meta.d_ellpack_addrs_ || !tt_meta.ellpack_on_device_) {
        throw std::runtime_error("copy_ldu_from_ellpack: ellpack not on device");
    }

    uint32_t aligned_cols = 32;
    auto cells = tt_meta.cell_count;
    auto allocEntries = tt::round_up(cells, 32) * aligned_cols;

    auto ellpack_addr = tt_meta.ellpack_addr_;
    auto dat_buf = new float[allocEntries];

    tt::tt_metal::EnqueueReadBuffer(
        device->command_queue(0),
        tt_meta.d_ellpack_vals_,
        dat_buf,
        true
    );

    auto get_ellpack_val = [&](uint32_t row, uint32_t col) { // still in not-really-tiled format (32x32 yes, 16x16 faces no)
        uint32_t* const ellpack_line_start = ellpack_addr + row*aligned_cols;
        uint32_t* const ellpack_line_end = ellpack_line_start + aligned_cols;
        for (uint32_t* cur_addr = ellpack_line_start; cur_addr != ellpack_line_end; cur_addr++) {
            uint32_t col_addr = *cur_addr;
            if (col_addr == UINT32_MAX) { // no more entries in line to search
                break;
            } else if (col_addr == col) { // found the entry
                return dat_buf[row*aligned_cols + (cur_addr - ellpack_line_start)];
            }
        }
        return 0.0f; // not in ellpack
    };

    bool diagNonZero = false;
    if (diagField) {
        float* diag = diagField->data();
        for (uint32_t i = 0; i < cells; ++i) {
            float val = get_ellpack_val(i, i);
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
                auto val = get_ellpack_val(upAddr, lowAddr);
                lower[i] = val;
                lowerNonZero |= (lower[i] != 0.0f);
            }
            if (upper) {
                auto val = get_ellpack_val(lowAddr, upAddr);
                upper[i] = val;
                upperNonZero |= (upper[i] != 0.0f);
            }
        }
    }

    delete[] dat_buf;

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
void copy_ldu_addrs_to_device(BufferPool& k, tt_ldu_meta& tt_meta, const Foam::lduMatrix* lduMat) {

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
            throw std::runtime_error("Reallocating address buffer, not implemented");
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


void copy_ldu_contents_to_device(BufferPool& k,tt_ldu_meta& tt_meta, const Foam::lduMatrix* lduMat, bool reserve_all) {
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

tt_ldu_meta& ensure_lduMat_on_device_as_ldu(BufferPool& k, const Foam::lduMatrix* lduMat, bool reserve_all_parts) {

    auto& tt_meta = get_tt_meta(lduMat, ldu_tt_meta_map);

    if (!tt_meta.addrs_on_device_) {
        #ifdef TRACY_ENABLE
        ZoneScopedN("TT Meta ldu copy addrs");
        #endif
        copy_ldu_addrs_to_device(k, tt_meta, lduMat);
    } else {
        #ifdef TRACY_ENABLE
        ZoneScopedN("TT Meta ldu reuse addrs");
        #endif
    }

    if (!tt_meta.contents_on_device_ ||
        (tt_meta.contents_on_device_ && (!reserve_all_parts ^ (tt_meta.lower_contents_start_ == tt_meta.upper_contents_start_)))
    ) { // if not uploaded, or if uploaded in a different format than requested now (i.e. reserve_all_parts changed) [we should be good with unreserved, unless we are running an operation that makes it asymmetric, in which case it is implicitly all reserved]
        #ifdef TRACY_ENABLE
        ZoneScopedN("TT Meta ldu copy contents");
        #endif
        copy_ldu_contents_to_device(k, tt_meta, lduMat, reserve_all_parts);
    } else {
        #ifdef TRACY_ENABLE
        ZoneScopedN("TT Meta ldu reuse contents");
        #endif
    }

    return tt_meta;
}

void copy_ldu_to_ellpack(
    BufferPool& bufferPool,
    tt_ldu_meta& tt_meta,
    const Foam::lduMatrix* lduMat
) {

    // Foam::Info << "Copying ellpack to device for " << reinterpret_cast<const void*>(lduMat) << Foam::endl;

    auto* device = bufferPool.device_;

    auto cells = lduMat->lduAddr().size();
    tt_meta.cell_count = cells;

    uint32_t aligned_cols = 32;

    auto allocEntries = tt::round_up(cells, 32)*aligned_cols;

    auto dat_buf = new float[allocEntries]; // currently in tiles, but without faces (32 elements per row then next row)
    auto addr_buf = tt_meta.ellpack_addr_on_device_? nullptr : new uint32_t[allocEntries]; // should be cached per mesh, not matrix
    auto col_counts = new uint32_t[cells];
    for (auto i = 0; i < cells; ++i) {
        col_counts[i] = 0;
    }

    if (!tt_meta.d_ellpack_vals_) {
        tt_meta.d_ellpack_vals_ = tt::tt_metal::CreateBuffer({
            .device = device,
            .size = sizeof(float)*allocEntries,
            .page_size = tt_metal::detail::TileSize(DataFormat::Float32),
            .buffer_type = tt::tt_metal::BufferType::DRAM
        });
    }

    if (!tt_meta.d_ellpack_addrs_) {
        tt_meta.d_ellpack_addrs_ = tt::tt_metal::CreateBuffer({
            .device = device,
            .size = sizeof(uint32_t)*allocEntries,
            .page_size = tt_metal::detail::TileSize(DataFormat::UInt32),
            .buffer_type = tt::tt_metal::BufferType::DRAM
        });
    }

    uint32_t max_cols = 0;
    uint32_t sum_cols = 0;

    auto get_next_col = [&](int row) {
        auto target = col_counts[row];

        if (target >= aligned_cols) {
            throw std::runtime_error("copy_ldu_to_ellpack: too many non-zeros in row " + std::to_string(row) + ", max is " + std::to_string(aligned_cols));
        }

        auto next = target + 1;

        if (next > max_cols) {
            max_cols = next;
        }
        sum_cols += 1;

        col_counts[row] = next;
        return target;
    };

    if (lduMat->hasLower() || lduMat->hasUpper()) { // need to do lower first, as any will be before diag in each row
        auto lowerAddr = lduMat->lduAddr().lowerAddr();
        auto upperAddr = lduMat->lduAddr().upperAddr();
        auto lower = lduMat->hasLower()? lduMat->lower().cdata() : lduMat->upper().cdata();
        auto sparse_vals = lowerAddr.size();

        for (int32_t i= 0; i < sparse_vals; ++i) {
            auto val = lower[i];
            if (val != 0.0f) {
                auto col_addr = lowerAddr[i];
                auto row_addr = upperAddr[i];
                auto next_col = get_next_col(row_addr);

                dat_buf[row_addr * aligned_cols + next_col] = lower[i];
                if (addr_buf) {
                    addr_buf[row_addr * aligned_cols + next_col] = col_addr;
                }
            }
        }
    }

    if (lduMat->hasDiag()) {
        auto diag = lduMat->diag().cdata();
        for (auto i = 0; i < cells; ++i) {
            auto val = diag[i];
            if (val != 0.0f) {
                auto next_col = get_next_col(i);
                dat_buf[i * aligned_cols + next_col] = val;
                if (addr_buf) {
                    addr_buf[i * aligned_cols + next_col] = i;
                }
            }
        }
    }

    if (lduMat->hasUpper()) {
        auto lowerAddr = lduMat->lduAddr().lowerAddr();
        auto upperAddr = lduMat->lduAddr().upperAddr();
        auto upper = lduMat->upper().cdata();
        auto sparse_vals = lowerAddr.size();

        for (int32_t i= 0; i < sparse_vals; ++i) {
            auto val = upper[i];
            if (val != 0.0f) {
                auto row_addr = lowerAddr[i];
                auto col_addr = upperAddr[i];
                auto next_col = get_next_col(row_addr);;

                dat_buf[row_addr * aligned_cols + next_col] = upper[i];
                if (addr_buf) {
                    addr_buf[row_addr * aligned_cols + next_col] = col_addr;
                }
            }
        }
    }

    if (addr_buf) { // fill with DontCare entries to allow  terminating list of values per line
        for (auto i = 0; i < cells; ++i) {
            for (auto j = col_counts[i]; j < aligned_cols; ++j) {
                addr_buf[i*aligned_cols + j] = UINT32_MAX;
                dat_buf[i*aligned_cols + j] = 0.0f; // so we can run it through tile-wide mat-mul
            }
        }
        for (auto i = cells; i < tt::round_up(cells, 32); ++i) { // clear the padding rows too
            addr_buf[i*aligned_cols + 0] = UINT32_MAX;
        }
    }

    tt_meta.ellpack_cols_ = max_cols;
    tt_meta.ellpack_avg_cols_ = float(sum_cols) / float(cells);

    #if TT_DEBUG > 0
    printf("ellpack mat %u x %u (max cols %u, avg cols %.2f):\n", cells, cells, max_cols, tt_meta.ellpack_avg_cols_);
    #if TT_DEBUG > 1
    auto print_addrs = addr_buf ? addr_buf : tt_meta.ellpack_addr_;
    for (uint32_t i = 0; i < static_cast<uint32_t>(cells); ++i) {
        printf("  %u: ", i);
        for (uint32_t j = 0; j < col_counts[i]; ++j) {
            printf("%3u:%6.3f ", print_addrs[i*aligned_cols + j], dat_buf[i*aligned_cols + j]);
        }
        printf("\n");
    }
    #endif
    #endif

    tt::tt_metal::EnqueueWriteBuffer(
        device->command_queue(0),
        tt_meta.d_ellpack_vals_,
        dat_buf,
        true  // we want to free the buffers
    );
    tt_meta.ellpack_on_device_ = true;

    if (addr_buf) {
        tt::tt_metal::EnqueueWriteBuffer(
            device->command_queue(0),
            tt_meta.d_ellpack_addrs_,
            addr_buf,
            false // source buffer lives on
        );
        tt_meta.ellpack_addr_ = addr_buf;
        tt_meta.ellpack_addr_on_device_ = true;
    }

    delete[] dat_buf;
    delete[] col_counts;
}

tt_ldu_meta& ensure_lduMat_on_device(
    BufferPool& k,
    const Foam::lduMatrix* lduMat,
    bool is_expand
) {

    #if TT_IMPL == TT_IMPL_LDU

        return ensure_lduMat_on_device_as_ldu(k, lduMat, is_expand);

    #elif TT_IMPL == TT_IMPL_DENSE

        auto& tt_meta = get_tt_meta(lduMat, ldu_tt_meta_map);

        if (!tt_meta.dense_on_device_) {
            copy_ldu_to_dense(k.device_, tt_meta, lduMat);
        }

        return tt_meta;

    #elif TT_IMPL == TT_IMPL_ELLPACK

        auto& tt_meta = get_tt_meta(lduMat, ldu_tt_meta_map);

        if (!tt_meta.ellpack_on_device_ || !tt_meta.ellpack_addr_on_device_) {
            copy_ldu_to_ellpack(k, tt_meta, lduMat);
        }

        return tt_meta;

    #else 

    #error Unknown TT IMPL TT_IMPL

    #endif
}

void copy_ldu_from_device(
    BufferPool& bufferPool,
    tt_ldu_meta& tt_meta,
    Foam::scalarField* diagField,
    Foam::scalarField* lowerField,
    Foam::scalarField* upperField,
    const Foam::lduAddressing& lduAddressing
) {

    #if TT_IMPL == TT_IMPL_LDU

        if (diagField) {
            tt::daisy::foam::copy_scalarField_from_device_bare(bufferPool, tt_meta.d_data_, diagField);
        }
        if (lowerField) {
            tt::daisy::foam::copy_scalarField_from_device_bare(bufferPool, *tt_meta.d_data_, lowerField, tt_meta.lower_contents_start_*4);
        }
        if (upperField) {
            tt::daisy::foam::copy_scalarField_from_device_bare(bufferPool, *tt_meta.d_data_, upperField, tt_meta.upper_contents_start_*4);
        }

    #elif TT_IMPL == TT_IMPL_DENSE

        copy_ldu_from_dense(bufferPool.device_, tt_meta, diagField, lowerField, upperField, lduAddressing);

    #elif TT_IMPL == TT_IMPL_ELLPACK

        copy_ldu_from_ellpack(bufferPool.device_, tt_meta, diagField, lowerField, upperField, lduAddressing);

    #else

    #error Unknown TT IMPL TT_IMPL

    #endif


}


std::tuple<tt_ldu_meta&, ReusableTtBuffer&, ReusableTtBuffer&> prepare_Amul_inputs(
    BufferPool& bufferPool,
    const Foam::lduMatrix& lduMat,
    const Foam::scalarField& psi,
    const Foam::scalarField& Apsi
) {

    auto& tt_meta = tt::daisy::foam::ensure_lduMat_on_device(bufferPool, &lduMat);

    #if TT_IMPL == TT_IMPL_LDU

    auto& tt_psi = tt::daisy::foam::copy_scalarField_to_device(bufferPool, psi);
    auto& tt_Apsi = bufferPool.allocateBuffer(sizeof(float)*Apsi.size(), tt::daisy::foam::tt_block_size);
    return {tt_meta, tt_psi, tt_Apsi};

    #elif TT_IMPL == TT_IMPL_DENSE

    auto& tt_psi = tt::daisy::foam::copy_scalarField_to_device_as_dense_mat(bufferPool, psi);
    auto& tt_Apsi = bufferPool.allocateBuffer(tt_psi.buffer->size(), tt_psi.buffer->page_size());

    return {tt_meta, tt_psi, tt_Apsi};

    #elif TT_IMPL == TT_IMPL_ELLPACK
    
    auto & tt_psi = tt::daisy::foam::copy_scalarField_to_device(bufferPool, psi);
    auto & tt_Apsi = bufferPool.allocateBuffer(tt_psi.buffer->size(), tt_psi.buffer->page_size());

    return {tt_meta, tt_psi, tt_Apsi};

    #else 

    #error Unknown TT IMPL TT_IMPL

    #endif
}

}  // namespace tt::daisy::foam