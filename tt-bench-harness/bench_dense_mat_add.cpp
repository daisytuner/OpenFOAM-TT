
#include <cstdlib>
#include <iostream>
#include <string>
#include <tt-metalium/host_api.hpp>

#include "dense_matBinOp.hpp"
#include "device_transfers.hpp"
#include "lduMatrix.H"
#include "lduPrimitiveMesh.H"
#include "ldu_meta_cache.hpp"
#include "messageStream.H"
#include "result_matchers.hpp"
#include "ttLduData.hpp"

using namespace tt::daisy;
using namespace tt::daisy::foam;

int main() {

    auto kernel_dir = std::string(std::getenv("TT_FOAM_KERNEL_DIR"));

    Foam::label cells = 4096;


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
    Foam::lduMatrix lduB(mesh);
    lduB.diag() = 2.0;
    lduB.lower() = 1.0;
    lduB.upper() = 150.0;

    Foam::lduMatrix lduRes(mesh);

    auto tt_meta_a = get_tt_meta(&lduA, ldu_tt_meta_map);
    auto tt_meta_b = get_tt_meta(&lduB, ldu_tt_meta_map);
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

    // tt::daisy::foam::copy_ldu_from_dense(device, tt_meta_a, &lduRes.diag(), &lduRes.lower(), &lduRes.upper(), lduRes.lduAddr());

    // bool fail = false;
    // if (!Foam::daisy::matches(lduA.diag(), lduRes.diag())) {
    //     Foam::SeriousError << "TT diag do not match!" << Foam::endl;
    //     Foam::Info << "org  Result: " << lduA.diag() << Foam::endl;
    //     Foam::Info << "new Result: " << lduRes.diag() << Foam::endl;
    //     fail = true;
    // }

    // if (!Foam::daisy::matches(lduA.lower(), lduRes.lower())) {
    //     Foam::SeriousError << "TT lower do not match!" << Foam::endl;
    //     Foam::Info << "org  Result: " << lduA.lower() << Foam::endl;
    //     Foam::Info << "new Result: " << lduRes.lower() << Foam::endl;
    //     fail = true;
    // }

    // if (!Foam::daisy::matches(lduA.upper(), lduRes.upper())) {
    //     Foam::SeriousError << "TT upper do not match!" << Foam::endl;
    //     Foam::Info << "org  Result: " << lduA.upper() << Foam::endl;
    //     Foam::Info << "new Result: " << lduRes.upper() << Foam::endl;
    //     fail = true;
    // }

    // if (fail) {
    //     throw new std::runtime_error("TT copy_ldu_to_dense / copy_ldu_from_dense results do not match!");
    // }

    
    tt::daisy::foam::copy_ldu_to_dense(device, tt_meta_b, &lduB);

    tt::tt_metal::Finish(device->command_queue(0));

    tt_launch_dense_matBinOp(
        device,
        *tt_meta_a.d_dense_,
        *tt_meta_b.d_dense_,
        *tt_meta_res.d_dense_,
        cells,
        "add",
        kernel_dir
    );

    tt::tt_metal::Finish(device->command_queue(0));

    tt_meta_res.dense_on_device_ = true;

    tt::daisy::foam::copy_ldu_from_dense(device, tt_meta_res, &lduRes.diag(), &lduRes.lower(), &lduRes.upper(), lduRes.lduAddr());

    tt::tt_metal::Finish(device->command_queue(0));

    Foam::Info << "Result: " << lduRes << Foam::endl;

    Foam::scalarField expected_diag(cells, 5.0);
    Foam::scalarField expected_lower(triang_size, 2.0);
    Foam::scalarField expected_upper(triang_size, 250.0);

    if (!Foam::daisy::matches(lduRes.diag(), expected_diag)) {
        Foam::SeriousError << "FAIL Expected diag: " << expected_diag << Foam::endl;
    }

    if (!Foam::daisy::matches(lduRes.lower(), expected_lower)) {
        Foam::SeriousError << "FAIL Expected lower: " << expected_lower << Foam::endl;
    }

    if (!Foam::daisy::matches(lduRes.upper(), expected_upper)) {
        Foam::SeriousError << "FAIL Expected upper: " << expected_upper << Foam::endl;
    }

    tt::tt_metal::CloseDevice(device);

    return 0;
}