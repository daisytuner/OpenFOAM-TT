/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2018 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Description
    Multiply a given vector (second argument) by the matrix or its transpose
    and return the result in the first argument.

\*---------------------------------------------------------------------------*/

#include "lduMatrix.H"

#ifdef __DAISY_INSTRUMENTATION
#include <daisy_rtl/daisy_rtl.h>
#endif
#include "messageStream.H"
#include "scalarField.H"
#include "result_matchers.hpp"

#ifdef ENABLE_TT
#include "kernel_launcher.hpp"
#include "ttLduData.hpp"
#include "device_transfers.hpp"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

void Foam::lduMatrix::Amul
(
    scalarField& Apsi,
    const tmp<scalarField>& tpsi,
    const FieldField<Field, scalar>& interfaceBouCoeffs,
    const lduInterfaceFieldPtrsList& interfaces,
    const direction cmpt
) const
{
#ifdef __DAISY_INSTRUMENTATION
    __daisy_metadata_t metadata = {
        .file_name = "lduMatrixATmul.cpp",
        .function_name = "Foam::lduMatrix::Amul",
        .line_begin = 38,
        .line_end = 119,
        .column_begin = 0,
        .column_end = 0,
        .region_name = "foam_lduMatrix_Amul",
    };
    unsigned long long region_id = __daisy_instrumentation_init(&metadata, __DAISY_EVENT_SET_CPU);

    __daisy_instrumentation_enter(region_id);
#endif
    
        scalar* __restrict__ ApsiPtr = Apsi.begin();

        const scalarField& psi = tpsi();

        // Initialise the update of interfaced interfaces
        initMatrixInterfaces
        (
            interfaceBouCoeffs,
            interfaces,
            psi,
            Apsi,
            cmpt
        );

        scalarField* tt_result;

        #ifdef ENABLE_TT
            #ifdef VERIFY_TT
                tt_result = new scalarField(Apsi.size(), 0.0);
            #else
                tt_result = &Apsi;
            #endif

            auto& k = require_kernel_launcher();

            auto& tt_meta = ensure_lduMat_on_device(k, this);

            auto& tt_psi = copy_scalarField_to_device(k, psi);
            auto& tt_Apsi = k.allocateBuffer(sizeof(float)*Apsi.size());
            auto [tt_iface_contents, iface_count] = copy_interfaceCoeffs_to_device(k, interfaceBouCoeffs, interfaces);

            k.launch_amul(
                tt_meta,
                *tt_psi.buffer,
                *tt_Apsi.buffer,
                // *tt_iface_contents.buffer,
                // iface_count,
                cmpt
            );

            copy_scalarField_from_device(k, tt_Apsi, tt_result);

            k.freeBuffer(tt_Apsi);
            k.freeBuffer(tt_psi);
            k.freeBuffer(tt_iface_contents);
        #endif

        #if !defined(ENABLE_TT) || defined(VERIFY_TT)

        const scalar* const __restrict__ psiPtr = psi.begin();

        const scalar* const __restrict__ diagPtr = diag().begin();

        const label* const __restrict__ uPtr = lduAddr().upperAddr().begin();
        const label* const __restrict__ lPtr = lduAddr().lowerAddr().begin();

        const scalar* const __restrict__ upperPtr = upper().begin();
        const scalar* const __restrict__ lowerPtr = lower().begin();

        const label nCells = diag().size();
        for (label cell=0; cell<nCells; cell++)
        {
            ApsiPtr[cell] = diagPtr[cell]*psiPtr[cell];
        }


        const label nFaces = upper().size();

        for (label face=0; face<nFaces; face++)
        {
            ApsiPtr[uPtr[face]] += lowerPtr[face]*psiPtr[lPtr[face]];
            ApsiPtr[lPtr[face]] += upperPtr[face]*psiPtr[uPtr[face]];
        }

        #if defined(ENABLE_TT) && defined(VERIFY_TT)
            if (!matches(*tt_result, Apsi)) {
                Foam::SeriousError << "Amul TT results do not match!" << Foam::endl;
                Foam::Info << "TT  Result: " << *tt_result << Foam::endl;
                Foam::Info << "CPU Result: " << Apsi << Foam::endl;
                Foam::Info << "Amul inVec: " << psi << Foam::endl;
                Foam::Info << "Amul matVec: " << *this << Foam::endl;
                Foam::Info << "Amul l_addr: " << lduAddr().lowerAddr() << Foam::endl;
                Foam::Info << "Amul u_addr: " << lduAddr().upperAddr() << Foam::endl;
                throw new std::runtime_error("Amul TT results do not match!");
            } else {
                Foam::Info << "Amul TT success" << Foam::endl;
                memcpy(ApsiPtr, tt_result->begin(), sizeof(scalar)*Apsi.size());
            }
        #endif

        // Update interface interfaces
        updateMatrixInterfaces
        (
            interfaceBouCoeffs,
            interfaces,
            psi,
            Apsi,
            cmpt
        );

        #endif

        tpsi.clear();
    
        #ifdef __DAISY_INSTRUMENTATION
        __daisy_instrumentation_exit(region_id);
        __daisy_instrumentation_finalize(region_id);
        #endif
}


void Foam::lduMatrix::Tmul
(
    scalarField& Tpsi,
    const tmp<scalarField>& tpsi,
    const FieldField<Field, scalar>& interfaceIntCoeffs,
    const lduInterfaceFieldPtrsList& interfaces,
    const direction cmpt
) const
{
#ifdef __DAISY_INSTRUMENTATION
    __daisy_metadata_t metadata = {
        .file_name = "lduMatrixATmul.cpp",
        .function_name = "Foam::lduMatrix::Tmul",
        .line_begin = 121,
        .line_end = 200,
        .column_begin = 0,
        .column_end = 0,
        .region_name = "foam_lduMatrix_Tmul",
                .loopnest_index = 0

    };
    unsigned long long region_id = __daisy_instrumentation_init(&metadata, __DAISY_EVENT_SET_CPU);

    __daisy_instrumentation_enter(region_id);
#endif


    scalar* __restrict__ TpsiPtr = Tpsi.begin();

    const scalarField& psi = tpsi();
    const scalar* const __restrict__ psiPtr = psi.begin();

    const scalar* const __restrict__ diagPtr = diag().begin();

    const label* const __restrict__ uPtr = lduAddr().upperAddr().begin();
    const label* const __restrict__ lPtr = lduAddr().lowerAddr().begin();

    const scalar* const __restrict__ lowerPtr = lower().begin();
    const scalar* const __restrict__ upperPtr = upper().begin();

    // Initialise the update of interfaced interfaces
    initMatrixInterfaces
    (
        interfaceIntCoeffs,
        interfaces,
        psi,
        Tpsi,
        cmpt
    );

    const label nCells = diag().size();
    for (label cell=0; cell<nCells; cell++)
    {
        TpsiPtr[cell] = diagPtr[cell]*psiPtr[cell];
    }

    const label nFaces = upper().size();
    for (label face=0; face<nFaces; face++)
    {
        TpsiPtr[uPtr[face]] += upperPtr[face]*psiPtr[lPtr[face]];
        TpsiPtr[lPtr[face]] += lowerPtr[face]*psiPtr[uPtr[face]];
    }

    // Update interface interfaces
    updateMatrixInterfaces
    (
        interfaceIntCoeffs,
        interfaces,
        psi,
        Tpsi,
        cmpt
    );

    tpsi.clear();

#ifdef __DAISY_INSTRUMENTATION
    __daisy_instrumentation_exit(region_id);
    __daisy_instrumentation_finalize(region_id);
#endif
}


void Foam::lduMatrix::sumA
(
    scalarField& sumA,
    const FieldField<Field, scalar>& interfaceBouCoeffs,
    const lduInterfaceFieldPtrsList& interfaces
) const
{
#ifdef __DAISY_INSTRUMENTATION
    __daisy_metadata_t metadata = {
        .file_name = "lduMatrixATmul.cpp",
        .function_name = "Foam::lduMatrix::sumA",
        .line_begin = 203,
        .line_end = 271,
        .column_begin = 0,
        .column_end = 0,
        .region_name = "foam_lduMatrix_sumA",
        .loopnest_index = 0
    };
    unsigned long long region_id = __daisy_instrumentation_init(&metadata, __DAISY_EVENT_SET_CPU);

    __daisy_instrumentation_enter(region_id);
#endif

    scalarField* tt_result;

    #ifdef ENABLE_TT
        #ifdef VERIFY_TT
            tt_result = new scalarField(sumA.size(), 0.0);
        #else
            tt_result = &sumA;
        #endif

        auto& k = require_kernel_launcher();

        auto& tt_meta = ensure_lduMat_on_device(k, this);

        auto& tt_res = k.allocateBuffer(sizeof(float)*sumA.size());
        auto [tt_iface_contents, iface_count] = copy_interfaceCoeffs_to_device(k, interfaceBouCoeffs, interfaces);

        k.launch_suma(
            tt_meta,
            *tt_res.buffer,
            *tt_iface_contents.buffer,
            iface_count
        );

        copy_scalarField_from_device(k, tt_res, tt_result);

        k.freeBuffer(tt_res);
        k.freeBuffer(tt_iface_contents);
    #endif

    #if !defined(ENABLE_TT) || defined(VERIFY_TT)

    scalar* __restrict__ sumAPtr = sumA.begin();

    const scalar* __restrict__ diagPtr = diag().begin();

    const label* __restrict__ uPtr = lduAddr().upperAddr().begin();
    const label* __restrict__ lPtr = lduAddr().lowerAddr().begin();

    const scalar* __restrict__ lowerPtr = lower().begin();
    const scalar* __restrict__ upperPtr = upper().begin();

    const label nCells = diag().size();
    const label nFaces = upper().size();

    for (label cell=0; cell<nCells; cell++)
    {
        sumAPtr[cell] = diagPtr[cell];
    }

    for (label face=0; face<nFaces; face++)
    {
        sumAPtr[uPtr[face]] += lowerPtr[face];
        sumAPtr[lPtr[face]] += upperPtr[face];
    }

    // Add the interface internal coefficients to diagonal
    // and the interface boundary coefficients to the sum-off-diagonal
    forAll(interfaces, patchi)
    {
        if (interfaces.set(patchi))
        {
            const labelUList& pa = lduAddr().patchAddr(patchi);
            const scalarField& pCoeffs = interfaceBouCoeffs[patchi];

            forAll(pa, face)
            {
                sumAPtr[pa[face]] -= pCoeffs[face];
            }
        }
    }

    #endif

    #if defined(ENABLE_TT) && defined(VERIFY_TT)
    if (!matches(*tt_result, sumA)) {
            Foam::SeriousError << "sumA TT results do not match!" << Foam::endl;
            Foam::Info << "TT  Result: " << *tt_result << Foam::endl;
            Foam::Info << "CPU Result: " << sumA << Foam::endl;
            Foam::Info << "sumA matVec: " << *this << Foam::endl;
            Foam::Info << "sumA l_addr: " << lduAddr().lowerAddr() << Foam::endl;
            Foam::Info << "sumA u_addr: " << lduAddr().upperAddr() << Foam::endl;
            Foam::Info << "sumA ifaceCoeffs: " << interfaceBouCoeffs << Foam::endl;
            throw new std::runtime_error("sumA TT results do not match!");
        } else {
            Foam::Info << "sumA TT success" << Foam::endl;
            memcpy(sumAPtr, tt_result->begin(), sizeof(scalar)*sumA.size());
        }
    #endif

#ifdef __DAISY_INSTRUMENTATION
    __daisy_instrumentation_exit(region_id);
    __daisy_instrumentation_finalize(region_id);
#endif
}


void Foam::lduMatrix::residual
(
    scalarField& rA,
    const scalarField& psi,
    const scalarField& source,
    const FieldField<Field, scalar>& interfaceBouCoeffs,
    const lduInterfaceFieldPtrsList& interfaces,
    const direction cmpt
) const
{
#ifdef __DAISY_INSTRUMENTATION
    __daisy_metadata_t metadata = {
        .file_name = "lduMatrixATmul.cpp",
        .function_name = "Foam::lduMatrix::residual",
        .line_begin = 274,
        .line_end = 371,
        .column_begin = 0,
        .column_end = 0,
        .region_name = "foam_lduMatrix_residual",
    };
    unsigned long long region_id = __daisy_instrumentation_init(&metadata, __DAISY_EVENT_SET_CPU);

    __daisy_instrumentation_enter(region_id);
#endif

    scalar* __restrict__ rAPtr = rA.begin();

    const scalar* const __restrict__ psiPtr = psi.begin();
    const scalar* const __restrict__ diagPtr = diag().begin();
    const scalar* const __restrict__ sourcePtr = source.begin();

    const label* const __restrict__ uPtr = lduAddr().upperAddr().begin();
    const label* const __restrict__ lPtr = lduAddr().lowerAddr().begin();

    const scalar* const __restrict__ upperPtr = upper().begin();
    const scalar* const __restrict__ lowerPtr = lower().begin();

    // Parallel boundary initialisation.
    // Note: there is a change of sign in the coupled
    // interface update.  The reason for this is that the
    // internal coefficients are all located at the l.h.s. of
    // the matrix whereas the "implicit" coefficients on the
    // coupled boundaries are all created as if the
    // coefficient contribution is of a source-kind (i.e. they
    // have a sign as if they are on the r.h.s. of the matrix.
    // To compensate for this, it is necessary to turn the
    // sign of the contribution.

    FieldField<Field, scalar> mBouCoeffs(interfaceBouCoeffs.size());

    forAll(mBouCoeffs, patchi)
    {
        if (interfaces.set(patchi))
        {
            mBouCoeffs.set(patchi, -interfaceBouCoeffs[patchi]);
        }
    }

    // Initialise the update of interfaced interfaces
    initMatrixInterfaces
    (
        mBouCoeffs,
        interfaces,
        psi,
        rA,
        cmpt
    );

    scalarField* tt_result;

    #ifdef ENABLE_TT
        #ifdef VERIFY_TT
            tt_result = new scalarField(rA.size());
        #else
            tt_result = &rA;
        #endif

        auto& k = require_kernel_launcher();

        auto& tt_meta = ensure_lduMat_on_device(k, this);

        auto& tt_psi = copy_scalarField_to_device(k, psi);
        auto& tt_source = copy_scalarField_to_device(k, source);
        auto& tt_res = k.allocateBuffer(sizeof(float)*rA.size());
        // auto [tt_iface_contents, iface_count] = copy_interfaceCoeffs_to_device(k, interfaceBouCoeffs, interfaces);

        k.launch_residual(
            tt_meta,
            *tt_psi.buffer,
            *tt_source.buffer,
            *tt_res.buffer,
            // *tt_iface_contents.buffer,
            // iface_count,
            cmpt
        );

        copy_scalarField_from_device(k, tt_res, tt_result);

        k.freeBuffer(tt_psi);
        k.freeBuffer(tt_source);
        k.freeBuffer(tt_res);
        // k.freeBuffer(tt_iface_contents);
    #endif

    #if !defined(ENABLE_TT) || defined(VERIFY_TT)

    const label nCells = diag().size();
    for (label cell=0; cell<nCells; cell++)
    {
        rAPtr[cell] = sourcePtr[cell] - diagPtr[cell]*psiPtr[cell];
    }


    const label nFaces = upper().size();

    for (label face=0; face<nFaces; face++)
    {
        rAPtr[uPtr[face]] -= lowerPtr[face]*psiPtr[lPtr[face]];
        rAPtr[lPtr[face]] -= upperPtr[face]*psiPtr[uPtr[face]];
    }

    #endif

    #if defined(ENABLE_TT) && defined(VERIFY_TT)
    if (!matches(*tt_result, rA)) {
            Foam::SeriousError << "residual TT results do not match!" << Foam::endl;
            Foam::Info << "TT  Result: " << *tt_result << Foam::endl;
            Foam::Info << "CPU Result: " << rA << Foam::endl;
            Foam::Info << "residual matVec: " << *this << Foam::endl;
            Foam::Info << "residual l_addr: " << lduAddr().lowerAddr() << Foam::endl;
            Foam::Info << "residual u_addr: " << lduAddr().upperAddr() << Foam::endl;
            Foam::Info << "residual psi: " << psi << Foam::endl;
            Foam::Info << "residual source: " << source << Foam::endl;
            // Foam::Info << "residual ifaceCoeffs: " << interfaceBouCoeffs << Foam::endl;
            throw new std::runtime_error("residual TT results do not match!");
        } else {
            Foam::Info << "residual TT success" << Foam::endl;
            memcpy(rAPtr, tt_result->begin(), sizeof(scalar)*rA.size());
        }
    #endif

    // Update interface interfaces
    updateMatrixInterfaces
    (
        mBouCoeffs,
        interfaces,
        psi,
        rA,
        cmpt
    );

#ifdef __DAISY_INSTRUMENTATION
    __daisy_instrumentation_exit(region_id);
    __daisy_instrumentation_finalize(region_id);
#endif
}


Foam::tmp<Foam::scalarField> Foam::lduMatrix::residual
(
    const scalarField& psi,
    const scalarField& source,
    const FieldField<Field, scalar>& interfaceBouCoeffs,
    const lduInterfaceFieldPtrsList& interfaces,
    const direction cmpt
) const
{
    tmp<scalarField> trA(new scalarField(psi.size()));
    residual(trA.ref(), psi, source, interfaceBouCoeffs, interfaces, cmpt);
    return trA;
}


Foam::tmp<Foam::scalarField > Foam::lduMatrix::H1() const
{
    tmp<scalarField > tH1
    (
        new scalarField(lduAddr().size(), 0.0)
    );

    if (lowerPtr_ || upperPtr_)
    {
        scalarField& H1_ = tH1.ref();

        scalar* __restrict__ H1Ptr = H1_.begin();

        const label* __restrict__ uPtr = lduAddr().upperAddr().begin();
        const label* __restrict__ lPtr = lduAddr().lowerAddr().begin();

        const scalar* __restrict__ lowerPtr = lower().begin();
        const scalar* __restrict__ upperPtr = upper().begin();

        const label nFaces = upper().size();

        for (label face=0; face<nFaces; face++)
        {
            H1Ptr[uPtr[face]] -= lowerPtr[face];
            H1Ptr[lPtr[face]] -= upperPtr[face];
        }
    }

    return tH1;
}


// ************************************************************************* //
