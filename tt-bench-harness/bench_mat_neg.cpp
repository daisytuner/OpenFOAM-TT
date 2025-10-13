
#include <cstdlib>
#include <iostream>
#include <string>
#include <tt-metalium/host_api.hpp>

#include "dense_matNeg.hpp"
#include "device_transfers.hpp"
#include "lduMatrix.H"
#include "lduPrimitiveMesh.H"
#include "ldu_meta_cache.hpp"
#include "messageStream.H"
#include "result_matchers.hpp"
#include "ttLduData.hpp"
#include "../tests/test_helpers.h"

using namespace tt::daisy;
using namespace tt::daisy::foam;

int main() {

    auto kernel_dir = std::string(std::getenv("TT_FOAM_KERNEL_DIR"));

    Foam::label cells = 128;


    auto triang_size = cells*(cells-1)/2;
    Foam::labelList addr_upper(triang_size);
    Foam::labelList addr_lower(triang_size);

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
            true
    );

    Foam::lduMatrix lduA(mesh);
    lduA.diag() = 3.0;
    lduA.lower() = 1.0;
    lduA.upper() = 100.0;

    Foam::lduMatrix lduRes(mesh);

    auto tt_meta_a = get_tt_meta(&lduA, ldu_tt_meta_map);
    auto tt_meta_res = get_tt_meta(&lduRes, ldu_tt_meta_map);

    tt::tt_metal::IDevice* device = tt::tt_metal::CreateDevice(0);

    auto all_cores = device->compute_with_storage_grid_size();

    tt_meta_res.cell_count = cells;
    
    tt_meta_res.d_dense_ = tt::tt_metal::CreateBuffer({
        .device = device,
        .size = tt::round_up(cells*cells*sizeof(float), 4096),
        .page_size = 4096,
        .buffer_type = tt::tt_metal::BufferType::DRAM
    });


    tt::daisy::foam::copy_ldu_to_dense(device, tt_meta_a, &lduA);

    tt::tt_metal::Finish(device->command_queue(0));

    tt_launch_dense_matNeg(
        device,
        *tt_meta_a.d_dense_,
        *tt_meta_res.d_dense_,
        cells,
        kernel_dir
    );

    tt::tt_metal::Finish(device->command_queue(0));

    tt_meta_res.dense_on_device_ = true;

    tt::daisy::foam::copy_ldu_from_dense(device, tt_meta_res, &lduRes.diag(), &lduRes.lower(), &lduRes.upper(), lduRes.lduAddr());

    tt::tt_metal::Finish(device->command_queue(0));

    tt::tt_metal::CloseDevice(device);

    idx = 0;
    for (Foam::label row = 0; row < cells; ++row) {
        for (Foam::label col = row + 1; col < cells; ++col) {
            EXPECT_FLOAT_EQ(lduA.upper()[idx] + lduRes.upper()[idx], 0.0f);
            EXPECT_FLOAT_EQ(lduA.lower()[idx] + lduRes.lower()[idx], 0.0f);

            ++idx;
        }
    }
    Foam::Info << "Result Diag:" << (lduRes.diag()+lduA.diag()) << Foam::endl;
    Foam::Info << "Input: " << lduA << Foam::endl;
    Foam::Info << "Result: " << lduRes << Foam::endl;

    std::cout << "Test passed!" << std::endl;

    return 0;
}