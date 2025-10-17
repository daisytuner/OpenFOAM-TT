#include "dense_matMul.hpp"
#include <cstdio>
#include <tt-metalium/kernel_types.hpp>
#include <tt-metalium/tensor_accessor_args.hpp>
#include <tt-metalium/work_split.hpp>
#include <tt-metalium/kernel_types.hpp>
#include "bmm_op.hpp"

namespace tt::daisy {

/**
 *   NOTE: Required in0_block_w change to support 1-wide inputs
 */
void tt_launch_dense_matMul_large(
    tt_metal::IDevice* device,
    tt_metal::Buffer& d_a,
    tt_metal::Buffer& d_b,
    tt_metal::Buffer& d_output,
    uint32_t M,
    uint32_t N,
    uint32_t K,
    uint32_t B,
    bool bcast_batch,
    std::filesystem::path kernel_dir
) {

    /*
     * Setup program to execute along with its buffers and kernels to use
     * Core range is just single core
     */
    tt_metal::Program program{};

    tt::DataFormat cb_data_format = tt::DataFormat::Float32;
    MathFidelity math_fidelity = MathFidelity::HiFi4;
    uint32_t single_tile_size = tt_metal::detail::TileSize(cb_data_format);
    // uint32_t single_tile_size = 2 * 1024;

    auto compute_with_storage_grid_size = device->compute_with_storage_grid_size();
    uint32_t num_cores_x = compute_with_storage_grid_size.x;
    uint32_t num_cores_y = compute_with_storage_grid_size.y;

    /*
     * EXtracting Matrix dimensions from input/output vectors
     */
    // C = A*B
    // MN = MK*KN
    uint32_t Mt = M / constants::TILE_HEIGHT;
    uint32_t Kt = K / constants::TILE_WIDTH;
    uint32_t Nt = N / constants::TILE_WIDTH;

    // NOTE: Only supports matmuls where output is blocks of 16 x 16 tiles (ie. multiples of 16*32 x 16*32)
    // NOTE: Maximum number of tiles in output is 120 * 16^2 = 30,720 (eg. [1, 1, 5120, 6144])2
    uint32_t in0_block_w = 2;
    // uint32_t out_subblock_h = 4;
    // uint32_t out_subblock_w = 2;
    // uint32_t per_core_M = 16;
    // uint32_t per_core_N = 16;

    // Get large matmul params
    auto matmul_params = bmm_op_utils::get_large_matmul_params(Mt, Nt, num_cores_y, num_cores_x, in0_block_w);
    uint32_t per_core_M = std::get<0>(matmul_params);
    uint32_t per_core_N = std::get<1>(matmul_params);
    uint32_t out_subblock_h = std::get<2>(matmul_params);
    uint32_t out_subblock_w = std::get<3>(matmul_params);

    fmt::print(" -- Metalium Core Sizing --\n");
    fmt::print(
        " -- per_core_M= {} -- per_core_N= {} -- out_subblock_h= {} -- out_subblock_w= {} --\n",
        per_core_M,
        per_core_N,
        out_subblock_h,
        out_subblock_w);

    TT_ASSERT(Mt % per_core_M == 0);
    TT_ASSERT(Nt % per_core_N == 0);
    TT_ASSERT(Kt % in0_block_w == 0);

    uint32_t in0_block_tiles = per_core_M * in0_block_w;
    uint32_t in0_CB_tiles = in0_block_tiles * 2;  // double buffer
    uint32_t in0_CB_size = in0_CB_tiles * single_tile_size;
    uint32_t in1_block_tiles = per_core_N * in0_block_w;
    uint32_t in1_CB_tiles = in1_block_tiles * 2;  // double buffer
    uint32_t in1_CB_size = in1_CB_tiles * single_tile_size;
    uint32_t out_block_tiles = per_core_M * per_core_N;
    uint32_t out_CB_tiles = out_block_tiles;  // No double buffer
    uint32_t out_CB_size = out_CB_tiles * single_tile_size;

    // Compute kernel compile time args
    uint32_t num_blocks = (Kt / in0_block_w);

    uint32_t in0_num_subblocks = (per_core_M / out_subblock_h);
    uint32_t in0_block_num_tiles = out_subblock_h * in0_block_w * in0_num_subblocks;
    uint32_t in0_subblock_num_tiles = out_subblock_h * in0_block_w;

    uint32_t in1_num_subblocks = (per_core_N / out_subblock_w);
    uint32_t in1_block_num_tiles = out_subblock_w * in0_block_w * in1_num_subblocks;
    uint32_t in1_per_core_w = out_subblock_w * in1_num_subblocks;

    uint32_t out_subblock_num_tiles = out_subblock_h * out_subblock_w;

    std::vector<uint32_t> compute_kernel_args = {
        in0_block_w,             // in0_block_w
        in0_num_subblocks,       // in0_num_subblocks
        in0_block_num_tiles,     // in0_block_num_tiles
        in0_subblock_num_tiles,  // in0_subblock_num_tiles

        in1_num_subblocks,    // in1_num_subblocks
        in1_block_num_tiles,  // in1_block_num_tiles
        in1_per_core_w,       // in1_per_core_w

        num_blocks,  // num_blocks

        out_subblock_h,          // out_subblock_h
        out_subblock_w,          // out_subblock_w
        out_subblock_num_tiles,  // out_subblock_num_tiles
        B                        // batch
    };

    /*
     * Multi-Core prep
     */
    // auto compute_with_storage_grid_size = device->compute_with_storage_grid_size();
    // uint32_t num_cores_x = compute_with_storage_grid_size.x;
    // uint32_t num_cores_y = compute_with_storage_grid_size.y;

    uint32_t num_blocks_y = Mt / per_core_M;
    uint32_t num_blocks_x = Nt / per_core_N;
    uint32_t num_blocks_total = num_blocks_y * num_blocks_x;
    TT_ASSERT(num_blocks_total <= num_cores_x * num_cores_y);
    CoreRangeSet all_cores(
        tt_metal::num_cores_to_corerangeset(num_blocks_x * num_blocks_y, compute_with_storage_grid_size, true));

    /*
     * Config of Circular Buffer in the device L1
     * input tiles count is = 2 because it's single tile process, and double-buffer
     */
    uint32_t src0_cb_index = CBIndex::c_0;  // 0
    tt_metal::CircularBufferConfig cb_src0_config = tt_metal::CircularBufferConfig(in0_CB_size, {{src0_cb_index, cb_data_format}})
                                              .set_page_size(src0_cb_index, single_tile_size);
    tt_metal::CreateCircularBuffer(program, all_cores, cb_src0_config);

    uint32_t src1_cb_index = CBIndex::c_1;  // 1
    tt_metal::CircularBufferConfig cb_src1_config = tt_metal::CircularBufferConfig(in1_CB_size, {{src1_cb_index, cb_data_format}})
                                              .set_page_size(src1_cb_index, single_tile_size);
    tt_metal::CreateCircularBuffer(program, all_cores, cb_src1_config);

    uint32_t output_cb_index = tt::CBIndex::c_16;
    uint32_t interm0_cb_index = 24;
    std::map<uint8_t, tt::DataFormat> output_cb_data_format_spec{
        {output_cb_index, cb_data_format}, {interm0_cb_index, cb_data_format}};
    tt_metal::CircularBufferConfig cb_output_config = tt_metal::CircularBufferConfig(out_CB_size, output_cb_data_format_spec)
                                                .set_page_size(output_cb_index, single_tile_size)
                                                .set_page_size(interm0_cb_index, single_tile_size);
    tt_metal::CreateCircularBuffer(program, all_cores, cb_output_config);

    /*
     * Compile time arguments
     */
    std::vector<uint32_t> reader_compile_time_args;
    tt_metal::TensorAccessorArgs(d_a).append_to(reader_compile_time_args);
    tt_metal::TensorAccessorArgs(d_b).append_to(reader_compile_time_args);

    std::vector<uint32_t> writer_compile_time_args;
    tt_metal::TensorAccessorArgs(d_output).append_to(writer_compile_time_args);

    /*
     * Create Kernels (Reader, Writer, Compute)
     */
    // Create reader and writer kernels per core
    auto reader_id = tt_metal::CreateKernel(
        program,
        kernel_dir / "dense" / "matmul_reuse" / "reader_bmm_tile_layout.cpp",
        all_cores,
        tt_metal::ReaderDataMovementConfig{
            reader_compile_time_args
        });

    auto writer_id = tt_metal::CreateKernel(
        program,
        kernel_dir / "dense" / "matmul_reuse" / "writer_bmm_tile_layout.cpp",
        all_cores,
        tt_metal::WriterDataMovementConfig{
            writer_compile_time_args
        });

    // Create compute kernel
    tt_metal::CreateKernel(
        program,
        kernel_dir / "dense" / "matmul_reuse" / "bmm_large_block_zm.cpp",
        all_cores,
        tt_metal::ComputeConfig {
            .math_fidelity = math_fidelity,
            .fp32_dest_acc_en = true,
            .compile_args = compute_kernel_args
        });

    /*
     * Kernels - Runtime arguments
     */
    uint32_t num_blocks_read = 0;
    for (int output_idx_y = 0; output_idx_y < num_blocks_y; output_idx_y++) {
        for (int output_idx_x = 0; output_idx_x < num_blocks_x; output_idx_x++) {
            int core_idx_x = num_blocks_read % num_cores_x;
            int core_idx_y = num_blocks_read / num_cores_x;
            CoreCoord core = {(std::size_t)core_idx_x, (std::size_t)core_idx_y};

            auto num_blocks = Kt / in0_block_w;

            // Write runtime args to device
            std::vector<uint32_t> mm_reader_args = {
                (std::uint32_t)d_a.address(),     // in0_tensor_addr
                (std::uint32_t)Kt * per_core_M * output_idx_y,  // in0_tensor_start_tile_id
                (std::uint32_t)1,                               // in0_tensor_stride_w
                (std::uint32_t)Kt,                              // in0_tensor_stride_h
                (std::uint32_t)in0_block_w,                     // in0_tensor_next_block_stride

                (std::uint32_t)in0_block_w,               // in0_block_w
                (std::uint32_t)per_core_M,                // in0_block_h
                (std::uint32_t)in0_block_w * per_core_M,  // in0_block_num_tiles

                (std::uint32_t)d_b.address(),  // in1_tensor_addr
                (std::uint32_t)per_core_N * output_idx_x,    // in1_tensor_start_tile_id
                (std::uint32_t)1,                            // in1_tensor_stride_w
                (std::uint32_t)Nt,                           // in1_tensor_stride_h
                (std::uint32_t)in0_block_w * Nt,             // in1_tensor_next_block_stride

                (std::uint32_t)per_core_N,                // in1_block_w
                (std::uint32_t)in0_block_w,               // in1_block_h
                (std::uint32_t)per_core_N * in0_block_w,  // in1_block_num_tiles

                (std::uint32_t)num_blocks,  // num_blocks

                (std::uint32_t)Mt * Kt,     // MtKt
                (std::uint32_t)Kt * Nt,     // KtNt
                (std::uint32_t)B,           // batch
                (std::uint32_t)bcast_batch  // bcast_B
            };

            printf("Core (%d,%d) does sub-blocks a: %dx%d, b: %dx%d, %d blocks\n", core_idx_x, core_idx_y, per_core_M, in0_block_w, in0_block_w, per_core_N, num_blocks);

            std::vector<uint32_t> writer_args = {
                (std::uint32_t)d_output.address(),                                  // out_buffer_addr
                (std::uint32_t)output_idx_x * per_core_N + output_idx_y * per_core_M * Nt,  // out_tensor_start_tile_id
                (std::uint32_t)1,                                                           // out_tensor_stride_w
                (std::uint32_t)Nt,                                                          // out_tensor_stride_h
                (std::uint32_t)out_subblock_w,       // out_tensor_next_subblock_stride_w
                (std::uint32_t)out_subblock_h * Nt,  // out_tensor_next_subblock_stride_h

                (std::uint32_t)out_subblock_w,                     // out_subblock_w
                (std::uint32_t)out_subblock_h,                     // out_subblock_h
                (std::uint32_t)(out_subblock_w * out_subblock_h),  // out_subblocks_w * out_subblocks_h
                (std::uint32_t)(per_core_N / out_subblock_w),      // out_num_subblocks_w
                (std::uint32_t)(per_core_M / out_subblock_h),      // out_num_subblocks_h

                (std::uint32_t)Mt * Nt,  // MtNt
                (std::uint32_t)B         // batch
            };

            tt_metal::SetRuntimeArgs(program, reader_id, core, mm_reader_args);
            tt_metal::SetRuntimeArgs(program, writer_id, core, writer_args);

            num_blocks_read++;
        }
    }

    EnqueueProgram(device->command_queue(0), program, false);
}

void tt_launch_dense_matMul_small(
    tt_metal::IDevice* device,
    tt_metal::Buffer& d_a,
    tt_metal::Buffer& d_b,
    tt_metal::Buffer& d_output,
    uint32_t M,
    uint32_t N,
    uint32_t K,
    uint32_t B,
    bool bcast_batch,
    std::filesystem::path kernel_dir
) {
    // Check if the configuration is valid - matrices must be divisible by tile dimensions
    TT_ASSERT(
        (M * N) % TILE_HW == 0,
        "Matrix dimensions M={} and N={} must be divisible by TILE_HW={} to use this matmul implementation",
        M,
        N,
        TILE_HW);

    // Setup the device and command queue for multi-core execution
    tt_metal::Program program{};

    // Get the compute grid size to determine how many cores are available
    auto core_grid = device->compute_with_storage_grid_size();
    auto num_output_tiles_total = (M * N) / TILE_HW;

    // Use the split_work_to_cores utility function to distribute matrix multiplication work
    // across available cores for efficient SPMD (Single Program, Multiple Data) execution.
    // This function takes the total number of output tiles and available cores, then calculates
    // how to divide the work when it cannot be evenly distributed. It returns two groups of cores:
    // - Primary group: handles more tiles per core
    // - Secondary group: handles fewer tiles per core
    // The secondary group is empty if the work can be evenly distributed across all cores. This
    // approach minimizes workload imbalance between cores for optimal performance.
    auto [num_cores, all_cores, core_group_1, core_group_2, work_per_core1, work_per_core2] =
        tt_metal::split_work_to_cores(core_grid, num_output_tiles_total);

    std::cout << "Using " << num_cores << " cores to process " << num_output_tiles_total << " tiles. ("
              << work_per_core1 << " on " << core_group_1.num_cores() << ", " << work_per_core2 << " on " << core_group_2.num_cores() << ")" << std::endl;

    // Extracting Matrix dimensions from input/output vectors and converting to tile coordinates.
    // The accelerator works with 32x32 tiles, so we need to convert from element dimensions
    // to tile dimensions for proper addressing and computation.
    const uint32_t Mt = M / TILE_HEIGHT;  // Number of tiles in M dimension
    const uint32_t Kt = K / TILE_WIDTH;   // Number of tiles in K dimension
    const uint32_t Nt = N / TILE_WIDTH;   // Number of tiles in N dimension

    // Create DRAM Buffers for input and output vectors.
    // We allocate DRAM buffers for the input matrices and output matrix.
    // Setting page_size to single_tile_size is the most common configuration for memory buffers in Metalium
    // as it is generic, works for most cases and achieves good performance.
    // Writing data from input vectors to source buffers.
    constexpr uint32_t single_tile_size = sizeof(float) * TILE_HEIGHT * TILE_WIDTH;  // 2 * 32 * 32 = 2048 bytes

    // Configure Circular Buffers
    // Circular buffers act as staging areas for data movement between DRAM and compute units.
    // Using 2 tiles per circular buffer to allow for double buffering (data movement can be reading from one tile while
    // the compute kernel is using the other tile). This number can be adjusted based on the use case, but generally
    // diminishing returns are observed after several tiles.
    // input tiles count is = 2 so one tile can be read while the other is being processed
    const auto cb_data_format = tt::DataFormat::Float32;
    uint32_t num_input_tiles = 2;
    tt_metal::CreateCircularBuffer(
        program,
        all_cores,  // create on all cores
        tt_metal::CircularBufferConfig(num_input_tiles * single_tile_size, {{CBIndex::c_0, cb_data_format}})
            .set_page_size(CBIndex::c_0, single_tile_size));

    tt_metal::CreateCircularBuffer(
        program,
        all_cores,  // create on all cores
        tt_metal::CircularBufferConfig(num_input_tiles * single_tile_size, {{CBIndex::c_1, cb_data_format}})
            .set_page_size(CBIndex::c_1, single_tile_size));

    tt_metal::CreateCircularBuffer(
        program,
        all_cores,  // create on all cores
        tt_metal::CircularBufferConfig(num_input_tiles * single_tile_size, {{CBIndex::c_16, cb_data_format}})
            .set_page_size(CBIndex::c_16, single_tile_size));

    // Create Kernels (Reader, Writer, Compute)
    // - Reader kernel: Handles reading input data from DRAM into circular buffers
    // - Writer kernel: Handles writing output data from circular buffers back to DRAM
    // - Compute kernel: Performs the actual matrix multiplication computation
    // All kernels run across all cores to enable parallel execution
    MathFidelity math_fidelity = MathFidelity::HiFi4;  // High fidelity math for accurate results
    std::vector<uint32_t> reader_compile_time_args;
    tt_metal::TensorAccessorArgs(d_a).append_to(reader_compile_time_args);
    tt_metal::TensorAccessorArgs(d_b).append_to(reader_compile_time_args);
    auto reader_id = tt_metal::CreateKernel(
        program,
        kernel_dir / "dense" / "matmul_multi" / "reader_mm_output_tiles_partitioned.cpp",
        all_cores,
        tt_metal::ReaderDataMovementConfig(
            reader_compile_time_args
        ));

    std::vector<uint32_t> writer_compile_time_args;
    tt_metal::TensorAccessorArgs(d_output).append_to(writer_compile_time_args);
    auto writer_id = tt_metal::CreateKernel(
        program,
        kernel_dir / "dense" / "matmul_multi" / "writer_unary_interleaved_start_id.cpp",
        all_cores,
        tt_metal::WriterDataMovementConfig(
            writer_compile_time_args
        ));

    auto compute_kernel_id = tt_metal::CreateKernel(
        program,
        kernel_dir / "dense" / "matmul_multi" / "mm.cpp",
        all_cores,
        tt_metal::ComputeConfig{
            .math_fidelity = math_fidelity,
            .fp32_dest_acc_en = true,
            .compile_args = {},
        });

    // Set Runtime Arguments for Kernels
    // Each core needs to know which portion of the work it's responsible for. We are parallelizing across output
    // tiles - each core computes different output tiles. Runtime arguments can be changed between program executions
    // without recompilation.
    uint32_t work_offset = 0;
    auto work_groups = {std::make_pair(core_group_1, work_per_core1), std::make_pair(core_group_2, work_per_core2)};

    // Iterate through each work group and assign work to cores
    for (const auto& [ranges, work_per_core] : work_groups) {
        for (const auto& range : ranges.ranges()) {
            for (const auto& core : range) {
                // Set arguments for the reader kernel (data input)
                tt_metal::SetRuntimeArgs(
                    program,
                    reader_id,
                    core,
                    {
                        d_a.address(),  // Address of matrix A in DRAM
                        d_b.address(),  // Address of matrix B in DRAM
                        Mt,                           // Number of tiles in M dimension
                        Kt,                           // Number of tiles in K dimension
                        Nt,                           // Number of tiles in N dimension
                        work_offset,                  // Starting offset for this core's work
                        work_per_core
                    });              // Amount of work for this core

                // Set arguments for the writer kernel (data output)
                tt_metal::SetRuntimeArgs(
                    program, writer_id, core, {
                        d_output.address(),
                        work_per_core,
                        work_offset
                    });

                // Set arguments for the compute kernel
                tt_metal::SetRuntimeArgs(
                    program,
                    compute_kernel_id,
                    core,
                    {
                        work_per_core,            // Amount of work for this core
                        Kt                        // Number of tiles in K dimension for dot product
                    });
                work_offset += work_per_core;  // Update offset for next core
            }
        }
    }

    EnqueueProgram(device->command_queue(0), program, false);
}

void tt_launch_dense_matMul( // warning: does NOT PAD
    tt_metal::IDevice* device,
    tt_metal::Buffer& d_a,
    tt_metal::Buffer& d_b,
    tt_metal::Buffer& d_output,
    uint32_t M,
    uint32_t N,
    uint32_t K,
    uint32_t B,
    bool bcast_batch,
    std::filesystem::path kernel_dir
) {
    
    bool large = false;

    uint32_t Mt = M / constants::TILE_HEIGHT;
    uint32_t Kt = K / constants::TILE_WIDTH;
    uint32_t Nt = N / constants::TILE_WIDTH;

    auto tiles = Nt * Mt;

    if (tiles >= 256 && Nt >= 2 && Mt >= 2) {
        large = true;
    }

    // if (large) {
        // tt_launch_dense_matMul_large(device, d_a, d_b, d_output, M, N, K, B, bcast_batch, kernel_dir); // wrong results for more than 1 tile. Is it because we change the in_block_w from 2 to 1?
    // } else {
        tt_launch_dense_matMul_small(device, d_a, d_b, d_output, M, N, K, B, bcast_batch, kernel_dir);
    // }
}

}
