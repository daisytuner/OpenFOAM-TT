#pragma once

#include "ReusableTtBuffer.hpp"
#include "kernel_launcher.hpp"
#include "lduAddressing.H"
#include <Field.H>
#include <scalarField.H>
#include <FieldField.H>
#include <lduInterfaceFieldPtrsList.H>

ReusableTtBuffer& copy_scalarField_to_device(KernelLauncher& kernelLauncher, const Foam::scalarField& field);

void copy_scalarField_from_device(
    KernelLauncher& kernelLauncher,
    std::variant<std::reference_wrapper<tt::tt_metal::Buffer>, std::shared_ptr<tt::tt_metal::Buffer>> buffer,
    Foam::scalarField* field,
    uint32_t buf_offset = 0
);

void copy_scalarField_from_device(
    KernelLauncher& kernelLauncher,
    ReusableTtBuffer& buffer,
    Foam::scalarField* field,
    uint32_t buf_offset = 0
);

std::pair<ReusableTtBuffer&, int> copy_interfaceCoeffs_to_device(
    KernelLauncher& kernelLauncher,
    const Foam::FieldField<Foam::Field, Foam::scalar>& interfaceCoeffs,
    const Foam::lduInterfaceFieldPtrsList& interfaces
);

void copy_ldu_addrs_to_device(
    KernelLauncher& kernelLauncher,
    tt_ldu_meta& tt_meta,
    const Foam::lduMatrix* lduMat
);

void copy_ldu_contents_to_device(
    KernelLauncher& kernelLauncher,
    tt_ldu_meta& tt_meta,
    const Foam::lduMatrix* lduMat,
    bool reserve_all = false
);

void copy_ldu_to_dense(
    tt::tt_metal::IDevice* device,
    tt_ldu_meta& tt_meta,
    const Foam::lduMatrix* lduMat
);

std::tuple<bool, bool, bool> copy_ldu_from_dense(
    tt::tt_metal::IDevice* device,
    tt_ldu_meta& tt_meta,
    Foam::scalarField* diagField,
    Foam::scalarField* lowerField,
    Foam::scalarField* upperField,
    const Foam::lduAddressing& lduAddressing
);