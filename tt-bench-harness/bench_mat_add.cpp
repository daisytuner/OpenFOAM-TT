
#include <iostream>
#include <tt-metalium/host_api.hpp>

#include "dense_matOpAssign.hpp"
#include "device_transfers.hpp"
#include "lduMatrix.H"
#include "lduPrimitiveMesh.H"
#include "scalarList.H"

int main() {

    Foam::label cells = 25;


    Foam::labelList addr_upper(190);
    Foam::labelList addr_lower(190);

    // Walk the upper triangle of a square matrix (excluding diagonal)
    int idx = 0;
    for (Foam::label row = 0; row < cells; ++row) {
        for (Foam::label col = row + 1; col < cells; ++col) {
            addr_upper[idx] = col;
            addr_lower[idx] = row;

            ++idx;
        }
    }

    Foam::lduPrimitiveMesh mesh(
            cells,
            addr_lower,
            addr_upper,
            0, // comm
            false
    );

    tt::tt_metal::IDevice* device;

    auto all_cores = device->compute_with_storage_grid_size();

    TTDenseMatOpAssignKernelMeta p;

    tt_setup_dense_matOpAssign_program(p, all_cores, "+", std::filesystem::path(std::getenv("TT_FOAM_KERNEL_DIR")));

    auto d_dense_A = tt::tt_metal::CreateBuffer({
        .device = device,
        .size = tt::round_up(cells*cells*sizeof(float), p.page_size),
        .page_size = p.page_size,
        .buffer_type = tt::tt_metal::BufferType::DRAM
    });

    auto d_dense_B = tt::tt_metal::CreateBuffer({
        .device = device,
        .size = tt::round_up(cells*cells*sizeof(float), p.page_size),
        .page_size = p.page_size,
        .buffer_type = tt::tt_metal::BufferType::DRAM
    });

    auto d_dense_res = tt::tt_metal::CreateBuffer({
        .device = device,
        .size = tt::round_up(cells*cells*sizeof(float), p.page_size),
        .page_size = p.page_size,
        .buffer_type = tt::tt_metal::BufferType::DRAM
    });


    copy_ldu_to_dense(lduA, d_dense_A); // libOpenFOAM (tt)

    tt::tt_metal::Finish(device->command_queue(0));
    
    tt_convert_ldu_to_dense(lduB, d_dense_B); // libOpenFOAM (tt)

    tt::tt_metal::Finish(device->command_queue(0));

    tt_launch_dense_matOpAssign(
        p,
        d_dense_A,
        d_dense_B,
        d_dense_res,
        cells
    );

    copy_

    return 0;
}