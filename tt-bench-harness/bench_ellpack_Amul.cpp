
#include <cstdlib>
#include <iostream>
#include <string>
#include <tt-metalium/host_api.hpp>

#include "dense_matBinOp.hpp"
#include "dense_matMul.hpp"
#include "device_transfers.hpp"
#include "ellpack_matVec.hpp"
#include "error.H"
#include "lduMatrix.H"
#include "lduPrimitiveMesh.H"
#include "ldu_meta_cache.hpp"
#include "messageStream.H"
#include "result_matchers.hpp"
#include "scalarField.H"
#include "tt-metalium/buffer.hpp"
#include "ttLduData.hpp"
#include "Field.H"
#include "tmp.H"

using namespace tt::daisy;
using namespace tt::daisy::foam;

void refAmul(
    const Foam::lduMatrix& lduMat,
    Foam::scalarField& Apsi,
    const Foam::tmp<Foam::scalarField>& tpsi
) {
    Foam::scalar* __restrict__ ApsiPtr = Apsi.begin();

    const Foam::scalarField& psi = tpsi();

    const Foam::scalar* const __restrict__ psiPtr = psi.begin();

    const Foam::scalar* const __restrict__ diagPtr = lduMat.diag().begin();

    const Foam::label* const __restrict__ uPtr = lduMat.lduAddr().upperAddr().begin();
    const Foam::label* const __restrict__ lPtr = lduMat.lduAddr().lowerAddr().begin();

    const Foam::scalar* const __restrict__ upperPtr = lduMat.upper().begin();
    const Foam::scalar* const __restrict__ lowerPtr = lduMat.lower().begin();

    const Foam::label nCells = lduMat.diag().size();
    for (Foam::label cell=0; cell<nCells; cell++)
    {
        ApsiPtr[cell] = diagPtr[cell]*psiPtr[cell];
    }


    const Foam::label nFaces = lduMat.upper().size();

    for (Foam::label face=0; face<nFaces; face++)
    {
        ApsiPtr[uPtr[face]] += lowerPtr[face]*psiPtr[lPtr[face]];
        ApsiPtr[lPtr[face]] += upperPtr[face]*psiPtr[uPtr[face]];
    }

    tpsi.clear();
}

int main() {

    auto kernel_dir = std::string(std::getenv("TT_FOAM_KERNEL_DIR"));
    tt::tt_metal::IDevice* device = tt::tt_metal::CreateDevice(0);

    auto Nx = 32;
    auto Ny = 32;
    Foam::label cells = Nx * Ny;


    auto triang_size = 4*cells;
    Foam::labelList addr_upper(triang_size);
    Foam::labelList addr_lower(triang_size);

    // Walk the upper triangle of a square matrix (excluding diagonal)
    int idx = 0;
    // OpenFOAM cell numbering: i + j*Nx
    for (Foam::label j = 0; j < Ny; ++j) {
        for (Foam::label i = 0; i < Nx; ++i) {
            Foam::label addr = i + j * Nx;

            if (i < Nx - 1) {
                addr_lower[idx] = addr;
                addr_upper[idx] = addr + 1;
                ++idx;
            }
            // South neighbor (j+1)
            addr_lower[idx] = addr;
            addr_upper[idx] = addr + Nx;
            ++idx;
        }
    }

    // Resize arrays to actual number of off-diagonal entries
    addr_lower.setSize(idx);
    addr_upper.setSize(idx);

    Foam::lduPrimitiveMesh mesh(
            cells,
            addr_lower,
            addr_upper,
            0, // comm
            true
    );

    Foam::lduMatrix lduA(mesh);
    lduA.diag() = 3.0;
    lduA.lower() = 0.0; // no values at all means mirrored from upper, 1.0 as well
    lduA.upper() = 1.0;

    Foam::scalarField inVec(cells, 2.0);
    for (int i = 0; i < cells; ++i) {
        inVec[i] = static_cast<float>(i+1);
    }
    Foam::scalarField result(cells, 0.0);


    Foam::Info << "lduA: " << lduA << Foam::endl;
//    Foam::Info << "Input: " << inVec << Foam::endl;

    auto tt_meta_a = get_tt_meta(&lduA, ldu_tt_meta_map);
    auto tile_size = tt::tt_metal::detail::TileSize(tt::DataFormat::Float32);
    auto cells_aligned = tt::round_up(cells, tt::constants::TILE_WIDTH);

    BufferPool buffer_pool(device);

    // copying starts

    tt::daisy::foam::copy_ldu_to_ellpack(buffer_pool, tt_meta_a, &lduA);

    tt::tt_metal::Finish(device->command_queue(0));

    auto& d_inVec = tt::daisy::foam::copy_scalarField_to_device_bare(buffer_pool, inVec);

    tt::tt_metal::Finish(device->command_queue(0));

    auto& d_resVec = buffer_pool.allocateBuffer(d_inVec.buffer->size(), d_inVec.buffer->page_size());

//    Foam::lduMatrix lduRes(mesh);
//
//    tt::daisy::foam::copy_ldu_from_ellpack(device, tt_meta_a, &lduRes.diag(), &lduRes.lower(), &lduRes.upper(), lduRes.lduAddr());
//
//    bool fail = false;
//    if (!Foam::daisy::matches(lduA.diag(), lduRes.diag())) {
//        Foam::SeriousError << "TT diag do not match!" << Foam::endl;
//        Foam::Info << "org  Result: " << lduA.diag() << Foam::endl;
//        Foam::Info << "new Result: " << lduRes.diag() << Foam::endl;
//        fail = true;
//    }
//
//    if (!Foam::daisy::matches(lduA.lower(), lduRes.lower())) {
//        Foam::SeriousError << "TT lower do not match!" << Foam::endl;
//        Foam::Info << "org  Result: " << lduA.lower() << Foam::endl;
//        Foam::Info << "new Result: " << lduRes.lower() << Foam::endl;
//        fail = true;
//    }
//
//    if (!Foam::daisy::matches(lduA.upper(), lduRes.upper())) {
//        Foam::SeriousError << "TT upper do not match!" << Foam::endl;
//        Foam::Info << "org  Result: " << lduA.upper() << Foam::endl;
//        Foam::Info << "new Result: " << lduRes.upper() << Foam::endl;
//        fail = true;
//    }
//
//    if (fail) {
//        throw new std::runtime_error("TT copy_ldu_to_dense / copy_ldu_from_dense results do not match!");
//    }


    tt_launch_ellpack_matVecOp(
        device,
        tt_meta_a,
        *d_inVec.buffer,
        *d_resVec.buffer,
        kernel_dir
    );

    tt::tt_metal::Finish(device->command_queue(0));

    tt::daisy::foam::copy_scalarField_from_device_bare(buffer_pool, d_resVec, &result);

    tt::tt_metal::Finish(device->command_queue(0));

    Foam::scalarField expected(cells);

    refAmul(lduA, expected, inVec);

    if (!Foam::daisy::matches(result, expected)) {
        Foam::SeriousError << "FAIL Expected: " << expected << Foam::endl;
        Foam::Info << "Result: " << result << Foam::endl;
    }

    tt::tt_metal::CloseDevice(device);

    return 0;
}