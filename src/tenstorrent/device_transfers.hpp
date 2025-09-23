#pragma once

#include "ReusableTtBuffer.hpp"
#include "kernel_launcher.hpp"
#include <Field.H>
#include <scalarField.H>
#include <FieldField.H>
#include <lduInterfaceFieldPtrsList.H>

ReusableTtBuffer& copy_scalarField_to_device(KernelLauncher& kernelLauncher, const Foam::scalarField& field);

void copy_scalarField_from_device(KernelLauncher& kernelLauncher, ReusableTtBuffer& buffer, Foam::scalarField* field);

std::pair<ReusableTtBuffer&, int> copy_interfaceCoeffs_to_device(
    KernelLauncher& kernelLauncher,
    const Foam::FieldField<Foam::Field, Foam::scalar>& interfaceCoeffs,
    const Foam::lduInterfaceFieldPtrsList& interfaces
);

void copy_ldu_addrs_to_device(KernelLauncher& KernelLauncher,tt_ldu_meta& tt_meta, const Foam::lduMatrix* lduMat);

void copy_ldu_contents_to_device(KernelLauncher& KernelLauncher,tt_ldu_meta& tt_meta, const Foam::lduMatrix* lduMat);