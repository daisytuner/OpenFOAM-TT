#include "ellpack_matVec_foam.hpp"
#include "tt-metalium/buffer.hpp"
#include "tt-metalium/tt_backend_api_types.hpp"

#include <cmath>
#include <cstdint>
#include <tt-metalium/host_api.hpp>
#include <tt-metalium/work_split.hpp>
#include <filesystem>
#include <tt-metalium/kernel_types.hpp>
#include <tt-metalium/tensor_accessor_args.hpp>
#include <tt-metalium/tt_metal.hpp>

namespace tt::daisy::foam {

#define TT_DEBUG 0

std::tuple<uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t> calculate_ellpack_matVec_metrics(
    tt::tt_metal::IDevice* device,
    const tt::daisy::tt_ldu_meta& tt_meta,
    EllpackHwImpl hwImpl
) {
    if (!tt_meta.ellpack_addr_ || tt_meta.cell_count == 0) {
        throw std::runtime_error("tt_ldu_meta does not have ellpack_addr_ set");
    }

    uint32_t ell_tile_page_size = tt_metal::detail::TileSize(tt::DataFormat::Float32);
    uint32_t ell_tiles_total = (tt_meta.cell_count + 31u) / 32u;
    uint32_t tiles_in_batch = hwImpl == EllpackHwImpl::FPU? 4u : 8u;

    uint64_t dram_bytes_wr = ell_tiles_total * 32 * sizeof(float); // multiples of 32 floats in result
    uint32_t vecs_per_page = vector_block_size/sizeof(float);
    uint32_t vector_page_size = vector_block_size;
    auto vec2page_shift = static_cast<uint32_t>(std::log2(vecs_per_page));

    auto avail_cores = device->compute_with_storage_grid_size();

    auto [num_cores, used_cores, core_group_1, core_group_2, work_per_core1, work_per_core2] =
        tt::tt_metal::split_work_to_cores(avail_cores, ell_tiles_total);

    uint64_t dram_bytes_rd = ell_tile_page_size*2 * ell_tiles_total; // mat + addr

    auto first_tile = 0u;
    for (int c = 0; c < num_cores; ++c) {
        auto tiles = c < core_group_1.num_cores() ? work_per_core1 : work_per_core2;
        auto end_tile = first_tile + tiles;
        for (uint32_t t = first_tile; t < end_tile; t += tiles_in_batch) {
            auto last_t = std::min(end_tile, t + tiles_in_batch) -1;
            auto in_batch = last_t + 1 - t;
            auto first_vec = tt_meta.ellpack_first_col_per_tile_[t];
            auto last_vec = tt_meta.ellpack_last_col_per_tile_[last_t];
            auto first_vec_page = first_vec >> vec2page_shift;
            auto last_vec_page = last_vec >> vec2page_shift;
            auto vec_pages = last_vec_page - first_vec_page + 1;
            dram_bytes_rd += vec_pages * vector_page_size;
            // std::cout << " dram_rd " << vec_pages << " vpages for t" << t << "..+" << in_batch << " on c" << c << std::endl;
        }
        first_tile += tiles;
    }

    // flops calculated:
    uint64_t mul_flops = hwImpl == EllpackHwImpl::FPU?
        ell_tiles_total * 32 * 32 * 32
        : tt_meta.ellpack_avg_cols_ * tt_meta.cell_count;
    uint64_t add_flops = hwImpl == EllpackHwImpl::FPU?
        ell_tiles_total * (32 * 32 * 32 -1)
        : std::max(tt_meta.ellpack_avg_cols_-1, 0.0f) * tt_meta.cell_count;
    

    uint64_t ellpack_mat_bytes = ell_tiles_total * ell_tile_page_size;
    uint64_t bare_vector_size = ell_tiles_total * 32 * sizeof(float);


    
    return {
        dram_bytes_rd,
        dram_bytes_wr,
        mul_flops,
        add_flops,
        ellpack_mat_bytes,
        bare_vector_size
    };
}

void tt_launch_ellpack_matVecOp(
    tt::tt_metal::IDevice* device,
    tt::daisy::tt_ldu_meta& tt_meta,
    tt::tt_metal::Buffer& d_inVec,
    tt::tt_metal::Buffer& d_resVec,
    const std::filesystem::path& kernel_dir,
    EllpackHwImpl hwImpl,
    size_t region_id
) {
    tt_launch_ellpack_matVecOp(
        device,
        tt_meta.cell_count,
        tt_meta.ellpack_cols_,
        *tt_meta.d_ellpack_vals_,
        *tt_meta.d_ellpack_addrs_,
        d_inVec,
        d_resVec,
        tt_meta.ellpack_first_col_per_tile_,
        tt_meta.ellpack_last_col_per_tile_,
        kernel_dir,
        hwImpl,
        region_id
    );
}

}   // namespace tt::daisy::foam
