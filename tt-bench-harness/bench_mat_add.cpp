
#include <cstdlib>
#include <iostream>
#include <tt-metalium/host_api.hpp>

#include "dense_matOpAssign.hpp"
#include "device_transfers.hpp"
#include "lduMatrix.H"
#include "lduPrimitiveMesh.H"
#include "ldu_meta_cache.hpp"
#include "ttLduData.hpp"

using namespace tt::daisy;
using namespace tt::daisy::foam;

int main() {

    auto kernel_dir = std::filesystem::path(std::getenv("TT_FOAM_KERNEL_DIR"));

    Foam::label cells = 32;


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

    Foam::lduMatrix lduA(mesh);
    lduA.diag() = 1.0;
    lduA.lower() = 2.0;
    lduA.upper() = 3.0;
    Foam::lduMatrix lduB(mesh);
    lduB.diag() = 4.0;
    lduB.lower() = 5.0;
    lduB.upper() = 6.0;

    Foam::lduMatrix lduRes(mesh);

    auto tt_meta_a = get_tt_meta(&lduA, ldu_tt_meta_map);
    auto tt_meta_b = get_tt_meta(&lduB, ldu_tt_meta_map);
    auto tt_meta_res = get_tt_meta(&lduRes, ldu_tt_meta_map);

    tt::tt_metal::IDevice* device;

    auto all_cores = device->compute_with_storage_grid_size();

    TTDenseMatOpAssignKernelMeta p;

    // auto d_dense_A = tt::tt_metal::CreateBuffer({
    //     .device = device,
    //     .size = tt::round_up(cells*cells*sizeof(float), p.page_size),
    //     .page_size = p.page_size,
    //     .buffer_type = tt::tt_metal::BufferType::DRAM
    // });

    // auto d_dense_B = tt::tt_metal::CreateBuffer({
    //     .device = device,
    //     .size = tt::round_up(cells*cells*sizeof(float), p.page_size),
    //     .page_size = p.page_size,
    //     .buffer_type = tt::tt_metal::BufferType::DRAM
    // });

    tt_meta_res.cell_count = cells;
    
    tt_meta_res.d_dense_ = tt::tt_metal::CreateBuffer({
        .device = device,
        .size = tt::round_up(cells*cells*sizeof(float), p.page_size),
        .page_size = p.page_size,
        .buffer_type = tt::tt_metal::BufferType::DRAM
    });


    tt::daisy::foam::copy_ldu_to_dense(device, tt_meta_a, &lduA);

    tt::tt_metal::Finish(device->command_queue(0));
    
    tt::daisy::foam::copy_ldu_to_dense(device, tt_meta_b, &lduB);

    tt::tt_metal::Finish(device->command_queue(0));

    tt_launch_dense_matOpAssign(
        device,
        *tt_meta_a.d_dense_,
        *tt_meta_b.d_dense_,
        *tt_meta_res.d_dense_,
        cells,
        "+",
        kernel_dir
    );

    tt_meta_res.dense_on_device_ = true;

    tt::daisy::foam::copy_ldu_from_dense(device, tt_meta_res, &lduRes.diag(), &lduRes.lower(), &lduRes.upper(), lduRes.lduAddr());

    return 0;
}