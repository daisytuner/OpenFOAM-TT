/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2020 OpenFOAM Foundation
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
    lduMatrix member operations.

\*---------------------------------------------------------------------------*/

#include "lduMatrix.H"
#include "result_matchers.hpp"

#ifdef __DAISY_INSTRUMENTATION
#include <daisy_rtl/daisy_rtl.h>
#endif

#ifdef ENABLE_TT
#include "kernel_launcher.hpp"
#include "ldu_meta_cache.hpp"
#include "device_transfers.hpp"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

void Foam::lduMatrix::sumDiag()
{
#ifdef __DAISY_INSTRUMENTATION
    __daisy_metadata_t metadata = {
        .file_name = "lduMatrixOperations.cpp",
        .function_name = "Foam::lduMatrix::sumDiag",
        .line_begin = 70,
        .line_end = 210,
        .column_begin = 0,
        .column_end = 0,
        .region_name = "foam_lduMatrix_sumDiag",
    };
    unsigned long long region_id = __daisy_instrumentation_init(&metadata, __DAISY_EVENT_SET_CPU);

    __daisy_instrumentation_enter(region_id);
#endif

    if (!lowerPtr_ && !upperPtr_)
    {
        return;
    }

    scalarField& Diag = diag();
    scalarField* tt_result;

    #ifdef ENABLE_TT
        #ifdef VERIFY_TT
            tt_result = new scalarField(Diag.size(), 0.0);
        #else
            tt_result = &Diag;
        #endif

        auto& k = tt::daisy::foam::require_kernel_launcher();

        auto& tt_meta = tt::daisy::foam::ensure_lduMat_on_device(k, this);

        k.launch_sumDiag(
            tt_meta
        );

        tt::daisy::foam::copy_scalarField_from_device(k, *tt_meta.d_data_, tt_result);
    #endif

    #if !defined(ENABLE_TT) || defined(VERIFY_TT)


    const scalarField& Lower = const_cast<const lduMatrix&>(*this).lower();
    const scalarField& Upper = const_cast<const lduMatrix&>(*this).upper();

    const labelUList& l = lduAddr().lowerAddr();
    const labelUList& u = lduAddr().upperAddr();

    for (label face=0; face<l.size(); face++)
    {
        Diag[l[face]] += Lower[face];
        Diag[u[face]] += Upper[face];
    }

    #endif

    #if defined(ENABLE_TT) && defined(VERIFY_TT)
        if (!daisy::matches(*tt_result, Diag)) {
            Foam::SeriousError << "sumDiag TT results do not match!" << Foam::endl;
            Foam::Info << "TT  Result: " << *tt_result << Foam::endl;
            Foam::Info << "CPU Result: " << diag() << Foam::endl;
            Foam::Info << "sumDiag matVec: " << *this << Foam::endl;
            Foam::Info << "sumDiag l_addr: " << lduAddr().lowerAddr() << Foam::endl;
            Foam::Info << "sumDiag u_addr: " << lduAddr().upperAddr() << Foam::endl;
            throw new std::runtime_error("sumDiag TT results do not match!");
        } else {
            // Foam::Info << "sumDiag TT success" << Foam::endl;
            memcpy(Diag.data(), tt_result->begin(), sizeof(scalar)*Diag.size());
            delete tt_result;
        }
    #endif

#ifdef __DAISY_INSTRUMENTATION
    __daisy_instrumentation_exit(region_id);
    __daisy_instrumentation_finalize(region_id);
#endif
}


void Foam::lduMatrix::negSumDiag()
{
#ifdef __DAISY_INSTRUMENTATION
    __daisy_metadata_t metadata = {
        .file_name = "lduMatrixOperations.cpp",
        .function_name = "Foam::lduMatrix::negSumDiag",
        .line_begin = 70,
        .line_end = 210,
        .column_begin = 0,
        .column_end = 0,
        .region_name = "foam_lduMatrix_negSumDiag",
    };
    unsigned long long region_id = __daisy_instrumentation_init(&metadata, __DAISY_EVENT_SET_CPU);

    __daisy_instrumentation_enter(region_id);
#endif

    if (!lowerPtr_ && !upperPtr_)
    {
        return;
    }

    scalarField& Diag = diag();
    scalarField* tt_result;

    #ifdef ENABLE_TT
        #ifdef VERIFY_TT
            tt_result = new scalarField(Diag.size(), 0.0);
        #else
            tt_result = &Diag;
        #endif

        auto& k = tt::daisy::foam::require_kernel_launcher();

        auto& tt_meta = tt::daisy::foam::ensure_lduMat_on_device(k, this);

        k.launch_negSumDiag(
            tt_meta
        );

        tt::daisy::foam::copy_scalarField_from_device(k, *tt_meta.d_data_, tt_result);
    #endif

    #if !defined(ENABLE_TT) || defined(VERIFY_TT)

    const scalarField& Lower = const_cast<const lduMatrix&>(*this).lower();
    const scalarField& Upper = const_cast<const lduMatrix&>(*this).upper();

    const labelUList& l = lduAddr().lowerAddr();
    const labelUList& u = lduAddr().upperAddr();

    for (label face=0; face<l.size(); face++)
    {
        Diag[l[face]] -= Lower[face];
        Diag[u[face]] -= Upper[face];
    }

    #endif

    #if defined(ENABLE_TT) && defined(VERIFY_TT)
        if (!daisy::matches(*tt_result, Diag)) {
            Foam::SeriousError << "negSumDiag TT results do not match!" << Foam::endl;
            Foam::Info << "TT  Result: " << *tt_result << Foam::endl;
            Foam::Info << "CPU Result: " << diag() << Foam::endl;
            Foam::Info << "negSumDiag matVec: " << *this << Foam::endl;
            Foam::Info << "negSumDiag l_addr: " << lduAddr().lowerAddr() << Foam::endl;
            Foam::Info << "negSumDiag u_addr: " << lduAddr().upperAddr() << Foam::endl;
            throw new std::runtime_error("negSumDiag TT results do not match!");
        } else {
            // Foam::Info << "negSumDiag TT success" << Foam::endl;
            memcpy(Diag.data(), tt_result->begin(), sizeof(scalar)*Diag.size());
            delete tt_result;
        }
    #endif

#ifdef __DAISY_INSTRUMENTATION
    __daisy_instrumentation_exit(region_id);
    __daisy_instrumentation_finalize(region_id);
#endif
}


void Foam::lduMatrix::sumMagOffDiag
(
    scalarField& sumOff
) const
{
#ifdef __DAISY_INSTRUMENTATION
    __daisy_metadata_t metadata = {
        .file_name = "lduMatrixOperations.cpp",
        .function_name = "Foam::lduMatrix::sumMagOffDiag",
        .line_begin = 70,
        .line_end = 210,
        .column_begin = 0,
        .column_end = 0,
        .region_name = "foam_lduMatrix_sumMagOffDiag",
    };
    unsigned long long region_id = __daisy_instrumentation_init(&metadata, __DAISY_EVENT_SET_CPU);

    __daisy_instrumentation_enter(region_id);
#endif


    if (!lowerPtr_ && !upperPtr_)
    {
        return;
    }

    const scalarField& Lower = const_cast<const lduMatrix&>(*this).lower();
    const scalarField& Upper = const_cast<const lduMatrix&>(*this).upper();

    const labelUList& l = lduAddr().lowerAddr();
    const labelUList& u = lduAddr().upperAddr();

    for (label face = 0; face < l.size(); face++)
    {
        sumOff[u[face]] += mag(Lower[face]);
        sumOff[l[face]] += mag(Upper[face]);
    }

#ifdef __DAISY_INSTRUMENTATION
    __daisy_instrumentation_exit(region_id);
    __daisy_instrumentation_finalize(region_id);
#endif
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

void Foam::lduMatrix::operator=(const lduMatrix& A)
{
    if (this == &A)
    {
        FatalError
            << "lduMatrix::operator=(const lduMatrix&) : "
            << "attempted assignment to self"
            << abort(FatalError);
    }



    if (A.lowerPtr_)
    {
        lower() = A.lower();
    }
    else if (lowerPtr_)
    {
        delete lowerPtr_;
        lowerPtr_ = nullptr;
    }

    if (A.upperPtr_)
    {
        upper() = A.upper();
    }
    else if (upperPtr_)
    {
        delete upperPtr_;
        upperPtr_ = nullptr;
    }

    if (A.diagPtr_)
    {
        diag() = A.diag();
    }

    #if ENABLE_TT
    tt::daisy::foam::clear_tt_meta(this, false, true);
    #endif
}


void Foam::lduMatrix::negate()
{
#ifdef __DAISY_INSTRUMENTATION
    __daisy_metadata_t metadata = {
        .file_name = "lduMatrixOperations.cpp",
        .function_name = "Foam::lduMatrix::negate",
        .line_begin = 70,
        .line_end = 210,
        .column_begin = 0,
        .column_end = 0,
        .region_name = "foam_lduMatrix_negate",
    };
    unsigned long long region_id = __daisy_instrumentation_init(&metadata, __DAISY_EVENT_SET_CPU);

    __daisy_instrumentation_enter(region_id);
#endif

    scalarField * tt_result_diag = nullptr,
        * tt_result_lower = nullptr,
        * tt_result_upper = nullptr;

    #ifdef ENABLE_TT
        #ifdef VERIFY_TT
            if (diagPtr_) {
                tt_result_diag = new scalarField(diagPtr_->size());
            }
            if (lowerPtr_) {
                tt_result_lower = new scalarField(lowerPtr_->size());
            }
            if (upperPtr_) {
                tt_result_upper = new scalarField(upperPtr_->size());
            }
        #else
            tt_result_diag = diagPtr_;
            tt_result_lower = lowerPtr_;
            tt_result_upper = upperPtr_;
        #endif

        auto& k = tt::daisy::foam::require_kernel_launcher();

        auto& tt_meta = tt::daisy::foam::ensure_lduMat_on_device(k, this);

        k.launch_negate(
            tt_meta
        );

        if (tt_result_diag) {
            tt::daisy::foam::copy_scalarField_from_device(k, *tt_meta.d_data_, tt_result_diag);
        }
        if (tt_result_lower) {
            tt::daisy::foam::copy_scalarField_from_device(k, *tt_meta.d_data_, tt_result_lower, tt_meta.lower_contents_start_*4);
        }
        if (tt_result_upper) {
            tt::daisy::foam::copy_scalarField_from_device(k, *tt_meta.d_data_, tt_result_upper, tt_meta.upper_contents_start_*4);
        }
    #endif

    #if !defined(ENABLE_TT) || defined(VERIFY_TT)

    if (lowerPtr_)
    {
        lowerPtr_->negate();
    }

    if (upperPtr_)
    {
        upperPtr_->negate();
    }

    if (diagPtr_)
    {
        diagPtr_->negate();
    }

    #endif

    #if defined(ENABLE_TT) && defined(VERIFY_TT)
        bool fail = false;
        if (diagPtr_) {
            if (!daisy::matches(*tt_result_diag, *diagPtr_)) {
                Foam::SeriousError << "negate diag TT results do not match!" << Foam::endl;
                Foam::Info << "TT  Result diag: " << *tt_result_diag << Foam::endl;
                Foam::Info << "CPU Result diag: " << *diagPtr_ << Foam::endl;
                fail = true;
            } else {
                memcpy(diagPtr_->data(), tt_result_diag->begin(), sizeof(scalar)*diagPtr_->size());
                delete tt_result_diag;
            }
        }
        if (lowerPtr_) {
            if (!daisy::matches(*tt_result_lower, *lowerPtr_)) {
                Foam::SeriousError << "negate lower TT results do not match!" << Foam::endl;
                Foam::Info << "TT  Result lower: " << *tt_result_lower << Foam::endl;
                Foam::Info << "CPU Result lower: " << *lowerPtr_ << Foam::endl;
                fail = true;
            } else {
                memcpy(lowerPtr_->data(), tt_result_lower->begin(), sizeof(scalar)*lowerPtr_->size());
                delete tt_result_lower;
            }
        }
        if (upperPtr_) {
            if (!daisy::matches(*tt_result_upper, *upperPtr_)) {
                Foam::SeriousError << "negate upper TT results do not match!" << Foam::endl;
                Foam::Info << "TT  Result upper: " << *tt_result_upper << Foam::endl;
                Foam::Info << "CPU Result upper: " << *upperPtr_ << Foam::endl;
                fail = true;
            } else {
                memcpy(upperPtr_->data(), tt_result_upper->begin(), sizeof(scalar)*upperPtr_->size());
                delete tt_result_upper;
            }
        }

        if (fail) {
            throw new std::runtime_error("negate TT results do not match!");
        }
    #endif

#ifdef __DAISY_INSTRUMENTATION
    __daisy_instrumentation_exit(region_id);
    __daisy_instrumentation_finalize(region_id);
#endif
}


void Foam::lduMatrix::operator+=(const lduMatrix& A)
{
#ifdef __DAISY_INSTRUMENTATION
    __daisy_metadata_t metadata = {
        .file_name = "lduMatrixOperations.cpp",
        .function_name = "Foam::lduMatrix::+=",
        .line_begin = 70,
        .line_end = 210,
        .column_begin = 0,
        .column_end = 0,
        .region_name = "foam_lduMatrix_+=",
    };
    unsigned long long region_id = __daisy_instrumentation_init(&metadata, __DAISY_EVENT_SET_CPU);

    __daisy_instrumentation_enter(region_id);
#endif

    scalarField * tt_result_diag  = nullptr,
                * tt_result_lower = nullptr,
                * tt_result_upper = nullptr;

    #ifdef ENABLE_TT
        if (A.diagPtr_ || A.lowerPtr_ || A.upperPtr_) { // if A is 0, there is nothing to do

            if (A.diagPtr_ && !diagPtr_) { // if we need to create new ones, create our temps
                tt_result_diag = new scalarField(lduAddr().size());
            }

            bool is_expand = false;
            
            if (!upperPtr_ && A.upperPtr_) {
                tt_result_upper = new scalarField(lduAddr().upperAddr().size());
                is_expand = true;
            }
            if (!lowerPtr_ && A.lowerPtr_) {
                tt_result_lower = new scalarField(lduAddr().lowerAddr().size());
                is_expand = true;
            }

            #ifdef VERIFY_TT
                if (diagPtr_ && !tt_result_diag) { // if there is not already a tt_result buffer, create a temp one
                    tt_result_diag = new scalarField(diagPtr_->size());
                }
                if (lowerPtr_ && !tt_result_lower) {
                    tt_result_lower = new scalarField(lowerPtr_->size());
                }
                if (upperPtr_ && !tt_result_upper) {
                    tt_result_upper = new scalarField(upperPtr_->size());
                }
            #else
                tt_result_diag = diagPtr_;
                if (!tt_result_lower) {
                    tt_result_lower = lowerPtr_;
                }
                if (!tt_result_upper) {
                    tt_result_upper = upperPtr_;
                }
            #endif

            auto& k = tt::daisy::foam::require_kernel_launcher();

            auto& tt_meta = ensure_lduMat_on_device(k, this, is_expand);
            auto& a_tt_meta = ensure_lduMat_on_device(k, &A);

            k.launch_matAddAssign(
                tt_meta,
                a_tt_meta
            );

            if (tt_result_diag) {
                copy_scalarField_from_device(k, *tt_meta.d_data_, tt_result_diag);
            }
            if (tt_result_lower) {
                copy_scalarField_from_device(k, *tt_meta.d_data_, tt_result_lower, tt_meta.lower_contents_start_*4);
            }
            if (tt_result_upper) {
                copy_scalarField_from_device(k, *tt_meta.d_data_, tt_result_upper, tt_meta.upper_contents_start_*4);
            }
        }
    #endif

    #if !defined(ENABLE_TT) || defined(VERIFY_TT)

    if (A.diagPtr_)
    {
        diag() += A.diag();
    }

    if (symmetric() && A.symmetric())
    {
        upper() += A.upper();
    }
    else if (symmetric() && A.asymmetric())
    {
        if (upperPtr_)
        {
            lower();
        }
        else
        {
            upper();
        }

        upper() += A.upper();
        lower() += A.lower();
    }
    else if (asymmetric() && A.symmetric())
    {
        if (A.upperPtr_)
        {
            lower() += A.upper();
            upper() += A.upper();
        }
        else
        {
            lower() += A.lower();
            upper() += A.lower();
        }

    }
    else if (asymmetric() && A.asymmetric())
    {
        lower() += A.lower();
        upper() += A.upper();
    }
    else if (diagonal())
    {
        if (A.upperPtr_)
        {
            upper() = A.upper();
        }

        if (A.lowerPtr_)
        {
            lower() = A.lower();
        }
    }
    else if (A.diagonal())
    {
    }
    else
    {
        if (debug > 1)
        {
            WarningInFunction
                << "Unknown matrix type combination" << nl
                << "    this :"
                << " diagonal:" << diagonal()
                << " symmetric:" << symmetric()
                << " asymmetric:" << asymmetric() << nl
                << "    A    :"
                << " diagonal:" << A.diagonal()
                << " symmetric:" << A.symmetric()
                << " asymmetric:" << A.asymmetric()
                << endl;
        }
    }

    #endif

    #ifdef ENABLE_TT
    #ifdef VERIFY_TT
        bool fail = false;
        if (tt_result_diag) {
            if (!daisy::matches(*tt_result_diag, *diagPtr_)) {
                Foam::SeriousError << "+= diag TT results do not match!" << Foam::endl;
                Foam::Info << "TT  Result diag: " << *tt_result_diag << Foam::endl;
                Foam::Info << "CPU Result diag: " << *diagPtr_ << Foam::endl;
                fail = true;
            } else if (diagPtr_) {
                memcpy(diagPtr_->data(), tt_result_diag->begin(), sizeof(scalar)*diagPtr_->size());
                delete tt_result_diag;
            } else {
                diagPtr_ = tt_result_diag;
            }
        }
        if (tt_result_lower) {
            if (!daisy::matches(*tt_result_lower, *lowerPtr_)) {
                Foam::SeriousError << "+= lower TT results do not match!" << Foam::endl;
                Foam::Info << "TT  Result lower: " << *tt_result_lower << Foam::endl;
                Foam::Info << "CPU Result lower: " << *lowerPtr_ << Foam::endl;
                fail = true;
            } else if (lowerPtr_) { // if tt_result is just a temp copy, save data, delete temp
                memcpy(lowerPtr_->data(), tt_result_lower->begin(), sizeof(scalar)*lowerPtr_->size());
                delete tt_result_lower;
            } else { // tt_result is the only instance of this data (new field), just use that for ldu
                lowerPtr_ = tt_result_lower;
            }
        }
        if (tt_result_upper) {
            if (!daisy::matches(*tt_result_upper, *upperPtr_)) {
                Foam::SeriousError << "+= upper TT results do not match!" << Foam::endl;
                Foam::Info << "TT  Result upper: " << *tt_result_upper << Foam::endl;
                Foam::Info << "CPU Result upper: " << *upperPtr_ << Foam::endl;
                fail = true;
            } else if (upperPtr_) {
                memcpy(upperPtr_->data(), tt_result_upper->begin(), sizeof(scalar)*upperPtr_->size());
                delete tt_result_upper;
            } else {
                upperPtr_ = tt_result_upper;
            }
        }

        if (fail) {
            Foam::Info << " A: diag " << !!A.diagPtr_ << " lower " << !!A.lowerPtr_ << " upper " << !!A.upperPtr_ << Foam::endl;
            Foam::Info << " tt_diag " << !!tt_result_diag << " tt_lower " << !!tt_result_lower << " tt_upper " << !!tt_result_upper << Foam::endl;
            throw new std::runtime_error("+= TT results in call do not match!");
        }
    #else
        if (tt_result_diag && !diagPtr_) {
            diagPtr_ = tt_result_diag;
        }
        if (tt_result_lower && !lowerPtr_) {
            lowerPtr_ = tt_result_lower;
        }
        if (tt_result_upper && !upperPtr_) {
            upperPtr_ = tt_result_upper;
        }
    #endif
    #endif

#ifdef __DAISY_INSTRUMENTATION
    __daisy_instrumentation_exit(region_id);
    __daisy_instrumentation_finalize(region_id);
#endif
}


void Foam::lduMatrix::operator-=(const lduMatrix& A)
{
#ifdef __DAISY_INSTRUMENTATION
    __daisy_metadata_t metadata = {
        .file_name = "lduMatrixOperations.cpp",
        .function_name = "Foam::lduMatrix::-=",
        .line_begin = 70,
        .line_end = 210,
        .column_begin = 0,
        .column_end = 0,
        .region_name = "foam_lduMatrix_-=",
    };
    unsigned long long region_id = __daisy_instrumentation_init(&metadata, __DAISY_EVENT_SET_CPU);

    __daisy_instrumentation_enter(region_id);
#endif

    scalarField * tt_result_diag  = nullptr,
                * tt_result_lower = nullptr,
                * tt_result_upper = nullptr;

    #ifdef ENABLE_TT

        if (A.diagPtr_ || A.lowerPtr_ || A.upperPtr_) { // if A is 0, there is nothing to do

            if (A.diagPtr_ && !diagPtr_) { // if we need to create new ones, create our temps
                tt_result_diag = new scalarField(lduAddr().size());
            }

            bool is_expand = false;
            
            if (!upperPtr_ && A.upperPtr_) {
                tt_result_upper = new scalarField(lduAddr().upperAddr().size());
                is_expand = true;
            }
            if (!lowerPtr_ && A.lowerPtr_) {
                tt_result_lower = new scalarField(lduAddr().lowerAddr().size());
                is_expand = true;
            }

            #ifdef VERIFY_TT
                if (diagPtr_ && !tt_result_diag) { // if there is not already a tt_result buffer, create a temp one
                    tt_result_diag = new scalarField(diagPtr_->size());
                }
                if (lowerPtr_ && !tt_result_lower) {
                    tt_result_lower = new scalarField(lowerPtr_->size());
                }
                if (upperPtr_ && !tt_result_upper) {
                    tt_result_upper = new scalarField(upperPtr_->size());
                }
            #else
                tt_result_diag = diagPtr_;
                if (!tt_result_lower) {
                    tt_result_lower = lowerPtr_;
                }
                if (!tt_result_upper) {
                    tt_result_upper = upperPtr_;
                }
            #endif

            auto& k = tt::daisy::foam::require_kernel_launcher();

            auto& tt_meta = tt::daisy::foam::ensure_lduMat_on_device(k, this, is_expand);
            auto& a_tt_meta = tt::daisy::foam::ensure_lduMat_on_device(k, &A);

            k.launch_matSubAssign(
                tt_meta,
                a_tt_meta
            );

            if (tt_result_diag) {
                tt::daisy::foam::copy_scalarField_from_device(k, *tt_meta.d_data_, tt_result_diag);
            }
            if (tt_result_lower) {
                tt::daisy::foam::copy_scalarField_from_device(k, *tt_meta.d_data_, tt_result_lower, tt_meta.lower_contents_start_*4);
            }
            if (tt_result_upper) {
                tt::daisy::foam::copy_scalarField_from_device(k, *tt_meta.d_data_, tt_result_upper, tt_meta.upper_contents_start_*4);
            }
        }
    #endif

    #if !defined(ENABLE_TT) || defined(VERIFY_TT)

    if (A.diagPtr_)
    {
        diag() -= A.diag();
    }

    if (symmetric() && A.symmetric())
    {
        upper() -= A.upper();
    }
    else if (symmetric() && A.asymmetric())
    {
        if (upperPtr_)
        {
            lower();
        }
        else
        {
            upper();
        }

        upper() -= A.upper();
        lower() -= A.lower();
    }
    else if (asymmetric() && A.symmetric())
    {
        if (A.upperPtr_)
        {
            lower() -= A.upper();
            upper() -= A.upper();
        }
        else
        {
            lower() -= A.lower();
            upper() -= A.lower();
        }

    }
    else if (asymmetric() && A.asymmetric())
    {
        lower() -= A.lower();
        upper() -= A.upper();
    }
    else if (diagonal())
    {
        if (A.upperPtr_)
        {
            upper() = -A.upper();
        }

        if (A.lowerPtr_)
        {
            lower() = -A.lower();
        }
    }
    else if (A.diagonal())
    {
    }
    else
    {
        if (debug > 1)
        {
            WarningInFunction
                << "Unknown matrix type combination" << nl
                << "    this :"
                << " diagonal:" << diagonal()
                << " symmetric:" << symmetric()
                << " asymmetric:" << asymmetric() << nl
                << "    A    :"
                << " diagonal:" << A.diagonal()
                << " symmetric:" << A.symmetric()
                << " asymmetric:" << A.asymmetric()
                << endl;
        }
    }

    #endif

    #ifdef ENABLE_TT
    #ifdef VERIFY_TT
        bool fail = false;
        if (tt_result_diag) {
            if (!daisy::matches(*tt_result_diag, *diagPtr_)) {
                Foam::SeriousError << "-= diag TT results do not match!" << Foam::endl;
                Foam::Info << "TT  Result diag: " << *tt_result_diag << Foam::endl;
                Foam::Info << "CPU Result diag: " << *diagPtr_ << Foam::endl;
                fail = true;
            } else if (diagPtr_) {
                memcpy(diagPtr_->data(), tt_result_diag->begin(), sizeof(scalar)*diagPtr_->size());
                delete tt_result_diag;
            } else {
                diagPtr_ = tt_result_diag;
            }
        }
        if (tt_result_lower) {
            if (!daisy::matches(*tt_result_lower, *lowerPtr_)) {
                Foam::SeriousError << "-= lower TT results do not match!" << Foam::endl;
                Foam::Info << "TT  Result lower: " << *tt_result_lower << Foam::endl;
                Foam::Info << "CPU Result lower: " << *lowerPtr_ << Foam::endl;
                fail = true;
            } else if (lowerPtr_) { // if tt_result is just a temp copy, save data, delete temp
                memcpy(lowerPtr_->data(), tt_result_lower->begin(), sizeof(scalar)*lowerPtr_->size());
                delete tt_result_lower;
            } else { // tt_result is the only instance of this data (new field), just use that for ldu
                lowerPtr_ = tt_result_lower;
            }
        }
        if (tt_result_upper) {
            if (!daisy::matches(*tt_result_upper, *upperPtr_)) {
                Foam::SeriousError << "-= upper TT results do not match!" << Foam::endl;
                Foam::Info << "TT  Result upper: " << *tt_result_upper << Foam::endl;
                Foam::Info << "CPU Result upper: " << *upperPtr_ << Foam::endl;
                fail = true;
            } else if (upperPtr_) {
                memcpy(upperPtr_->data(), tt_result_upper->begin(), sizeof(scalar)*upperPtr_->size());
                delete tt_result_upper;
            } else {
                upperPtr_ = tt_result_upper;
            }
        }

        if (fail) {
            Foam::Info << " A: diag " << !!A.diagPtr_ << " lower " << !!A.lowerPtr_ << " upper " << !!A.upperPtr_ << Foam::endl;
            Foam::Info << " tt_diag " << !!tt_result_diag << " tt_lower " << !!tt_result_lower << " tt_upper " << !!tt_result_upper << Foam::endl;
            throw new std::runtime_error("-= TT results do not match!");
        }
    #else
        if (tt_result_diag && !diagPtr_) {
            diagPtr_ = tt_result_diag;
        }
        if (tt_result_lower && !lowerPtr_) {
            lowerPtr_ = tt_result_lower;
        }
        if (tt_result_upper && !upperPtr_) {
            upperPtr_ = tt_result_upper;
        }
    #endif
    #endif

#ifdef __DAISY_INSTRUMENTATION
    __daisy_instrumentation_exit(region_id);
    __daisy_instrumentation_finalize(region_id);
#endif
}


void Foam::lduMatrix::operator*=(const scalarField& sf)
{
#ifdef __DAISY_INSTRUMENTATION
    __daisy_metadata_t metadata = {
        .file_name = "lduMatrixOperations.cpp",
        .function_name = "Foam::lduMatrix::*=",
        .line_begin = 70,
        .line_end = 210,
        .column_begin = 0,
        .column_end = 0,
        .region_name = "foam_lduMatrix_*=",
    };
    unsigned long long region_id = __daisy_instrumentation_init(&metadata, __DAISY_EVENT_SET_CPU);

    __daisy_instrumentation_enter(region_id);
#endif

    if (diagPtr_)
    {
        *diagPtr_ *= sf;
    }

    // Non-uniform scaling causes a symmetric matrix
    // to become asymmetric
    if (symmetric() || asymmetric())
    {
        scalarField& upper = this->upper();
        scalarField& lower = this->lower();

        const labelUList& l = lduAddr().lowerAddr();
        const labelUList& u = lduAddr().upperAddr();

        for (label face=0; face<upper.size(); face++)
        {
            upper[face] *= sf[l[face]];
        }

        for (label face=0; face<lower.size(); face++)
        {
            lower[face] *= sf[u[face]];
        }
    }

    #if ENABLE_TT
    tt::daisy::foam::clear_tt_meta(this, false, true);
    #endif

#ifdef __DAISY_INSTRUMENTATION
    __daisy_instrumentation_exit(region_id);
    __daisy_instrumentation_finalize(region_id);
#endif
}


void Foam::lduMatrix::operator*=(scalar s)
{
#ifdef __DAISY_INSTRUMENTATION
    __daisy_metadata_t metadata = {
        .file_name = "lduMatrixOperations.cpp",
        .function_name = "Foam::lduMatrix::*=_scalar",
        .line_begin = 70,
        .line_end = 210,
        .column_begin = 0,
        .column_end = 0,
        .region_name = "foam_lduMatrix_*=_scalar",
    };
    unsigned long long region_id = __daisy_instrumentation_init(&metadata, __DAISY_EVENT_SET_CPU);

    __daisy_instrumentation_enter(region_id);
#endif

    if (diagPtr_)
    {
        *diagPtr_ *= s;
    }

    if (upperPtr_)
    {
        *upperPtr_ *= s;
    }

    if (lowerPtr_)
    {
        *lowerPtr_ *= s;
    }

    #if ENABLE_TT
    tt::daisy::foam::clear_tt_meta(this, false, true);
    #endif

#ifdef __DAISY_INSTRUMENTATION
    __daisy_instrumentation_exit(region_id);
    __daisy_instrumentation_finalize(region_id);
#endif
}


void Foam::lduMatrix::operator/=(const scalarField& sf)
{
#ifdef __DAISY_INSTRUMENTATION
    __daisy_metadata_t metadata = {
        .file_name = "lduMatrixOperations.cpp",
        .function_name = "Foam::lduMatrix::/=",
        .line_begin = 70,
        .line_end = 210,
        .column_begin = 0,
        .column_end = 0,
        .region_name = "foam_lduMatrix_/=",
    };
    unsigned long long region_id = __daisy_instrumentation_init(&metadata, __DAISY_EVENT_SET_CPU);

    __daisy_instrumentation_enter(region_id);
#endif

    if (diagPtr_)
    {
        *diagPtr_ /= sf;
    }

    // Non-uniform scaling causes a symmetric matrix
    // to become asymmetric
    if (symmetric() || asymmetric())
    {
        scalarField& upper = this->upper();
        scalarField& lower = this->lower();

        const labelUList& l = lduAddr().lowerAddr();
        const labelUList& u = lduAddr().upperAddr();

        for (label face=0; face<upper.size(); face++)
        {
            upper[face] /= sf[l[face]];
        }

        for (label face=0; face<lower.size(); face++)
        {
            lower[face] /= sf[u[face]];
        }
    }

    #if ENABLE_TT
    tt::daisy::foam::clear_tt_meta(this, false, true);
    #endif

#ifdef __DAISY_INSTRUMENTATION
    __daisy_instrumentation_exit(region_id);
    __daisy_instrumentation_finalize(region_id);
#endif
}


void Foam::lduMatrix::operator/=(scalar s)
{
#ifdef __DAISY_INSTRUMENTATION
    __daisy_metadata_t metadata = {
        .file_name = "lduMatrixOperations.cpp",
        .function_name = "Foam::lduMatrix::/=_scalar",
        .line_begin = 70,
        .line_end = 210,
        .column_begin = 0,
        .column_end = 0,
        .region_name = "foam_lduMatrix_/=_scalar",
    };
    unsigned long long region_id = __daisy_instrumentation_init(&metadata, __DAISY_EVENT_SET_CPU);

    __daisy_instrumentation_enter(region_id);
#endif

    if (diagPtr_)
    {
        *diagPtr_ /= s;
    }

    if (upperPtr_)
    {
        *upperPtr_ /= s;
    }

    if (lowerPtr_)
    {
        *lowerPtr_ /= s;
    }

    #if ENABLE_TT
    tt::daisy::foam::clear_tt_meta(this, false, true);
    #endif

#ifdef __DAISY_INSTRUMENTATION
    __daisy_instrumentation_exit(region_id);
    __daisy_instrumentation_finalize(region_id);
#endif
}


// ************************************************************************* //
