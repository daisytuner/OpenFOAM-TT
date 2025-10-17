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

#ifdef ENABLE_DAISY_RTL
#include <daisy_rtl/daisy_rtl.h>
#endif

#ifdef ENABLE_TT
#include "kernel_launcher.hpp"
#include "ldu_meta_cache.hpp"
#include "device_transfers.hpp"
#include "dense_matBinOp.hpp"

#if TT_IMPL == TT_IMPL_LDU
    #define ENABLE_TT_SUMDIAG
    #define ENABLE_TT_NEGSUMDIAG
#endif
#if TT_IMPL == TT_IMPL_LDU || TT_IMPL == TT_IMPL_DENSE
    #define ENABLE_TT_NEGATE
    #define ENABLE_TT_ADD_ASSIGN
    #define ENABLE_TT_SUB_ASSIGN
#endif
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

void Foam::lduMatrix::sumDiag()
{
    if (!lowerPtr_ && !upperPtr_)
    {
        return;
    }

    scalarField& Diag = diag();
    scalarField* tt_result;

    #ifdef ENABLE_TT_SUMDIAG
        #ifdef VERIFY_TT
            tt_result = new scalarField(Diag.size(), 0.0);
        #else
            tt_result = &Diag;
        #endif

        auto& k = tt::daisy::foam::require_kernel_launcher();

        auto& tt_meta = tt::daisy::foam::ensure_lduMat_on_device_as_ldu(k, this);

        k.launch_sumDiag(
            tt_meta
        );

        tt::daisy::foam::copy_scalarField_from_device_bare(k, tt_meta.d_data_, tt_result);
    #endif

    #if !defined(ENABLE_TT_SUMDIAG) || defined(VERIFY_TT)


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

    #if defined(ENABLE_TT_SUMDIAG) && defined(VERIFY_TT)
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
}


void Foam::lduMatrix::negSumDiag()
{
    if (!lowerPtr_ && !upperPtr_)
    {
        return;
    }

    scalarField& Diag = diag();
    scalarField* tt_result;

    #ifdef ENABLE_TT_NEGSUMDIAG
        #ifdef VERIFY_TT
            tt_result = new scalarField(Diag.size(), 0.0);
        #else
            tt_result = &Diag;
        #endif

        auto& k = tt::daisy::foam::require_kernel_launcher();

        auto& tt_meta = tt::daisy::foam::ensure_lduMat_on_device_as_ldu(k, this);

        k.launch_negSumDiag(
            tt_meta
        );

        tt::daisy::foam::copy_scalarField_from_device_bare(k, *tt_meta.d_data_, tt_result);
    #endif

    #if !defined(ENABLE_TT_NEGSUMDIAG) || defined(VERIFY_TT)

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

    #if defined(ENABLE_TT_NEGSUMDIAG) && defined(VERIFY_TT)
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
}


void Foam::lduMatrix::sumMagOffDiag
(
    scalarField& sumOff
) const
{
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
    scalarField * tt_result_diag = nullptr,
        * tt_result_lower = nullptr,
        * tt_result_upper = nullptr;

    bool had_diag = diagPtr_ != nullptr, had_lower = lowerPtr_ != nullptr, had_upper = upperPtr_ != nullptr;

    #ifdef ENABLE_TT_NEGATE
        #if TT_IMPL != TT_IMPL_LDU
        const bool force_full = true;
        #else
        const bool force_full = false;
        #endif
        #ifdef VERIFY_TT
            auto& addr = lduAddr();
            if (diagPtr_ || force_full) {
                tt_result_diag = new scalarField(addr.size());
            }
            if (lowerPtr_ || force_full) {
                tt_result_lower = new scalarField(addr.lowerAddr().size());
            }
            if (upperPtr_ || force_full) {
                tt_result_upper = new scalarField(addr.upperAddr().size());
            }
        #else
            tt_result_diag = force_full ? diag() : diagPtr_;
            tt_result_lower = force_full ? lower() : lowerPtr_;
            tt_result_upper = force_full ? upper() : upperPtr_;
        #endif

        auto& k = tt::daisy::foam::require_kernel_launcher();

        auto& tt_meta = tt::daisy::foam::ensure_lduMat_on_device(k, this);

        tt::daisy::foam::tt_compute_negate(
            k,
            tt_meta
        );

        tt::daisy::foam::copy_ldu_from_device(
            k,
            tt_meta,
            tt_result_diag,
            tt_result_lower,
            tt_result_upper,
            lduAddr()
        );
    #endif

    #if !defined(ENABLE_TT_NEGATE) || defined(VERIFY_TT)

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

    #if defined(ENABLE_TT_NEGATE) && defined(VERIFY_TT)
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
            Foam::Info << "had diag: " << had_diag << ", had lower: " << had_lower << ", had upper: " << had_upper << Foam::endl;
            Foam::Info << " => " << (force_full? "force " : "") << " diag: " << (tt_result_diag != nullptr) << ", lower: " << (tt_result_lower != nullptr) << ", upper: " << (tt_result_upper != nullptr) << Foam::endl;
            throw new std::runtime_error("negate TT results do not match!");
        }
    #endif
}


void Foam::lduMatrix::operator+=(const lduMatrix& A)
{
    scalarField * tt_result_diag  = nullptr,
                * tt_result_lower = nullptr,
                * tt_result_upper = nullptr;

    bool had_diag = diagPtr_ != nullptr, had_lower = lowerPtr_ != nullptr, had_upper = upperPtr_ != nullptr;

    #ifdef ENABLE_TT_ADD_ASSIGN
        if (A.diagPtr_ || A.lowerPtr_ || A.upperPtr_) { // if A is 0, there is nothing to do

            #if (TT_IMPL != TT_IMPL_LDU)
            const bool force_full = true;
            #else
            const bool force_full = false;
            #endif

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
                if (!tt_result_diag) {
                    tt_result_diag = diagPtr_;
                }
                if (!tt_result_lower) {
                    tt_result_lower = lowerPtr_;
                }
                if (!tt_result_upper) {
                    tt_result_upper = upperPtr_;
                }
            #endif
            if (!tt_result_diag && force_full) {
                tt_result_diag = new scalarField(lduAddr().size());
            }
            if (!tt_result_lower && force_full) {
                tt_result_lower = new scalarField(lduAddr().lowerAddr().size());
            }
            if (!tt_result_upper && force_full) {
                tt_result_upper = new scalarField(lduAddr().upperAddr().size());
            }

            auto& k = tt::daisy::foam::require_kernel_launcher();

            auto& tt_meta = ensure_lduMat_on_device(k, this, is_expand);
            auto& a_tt_meta = ensure_lduMat_on_device(k, &A);

            tt::daisy::foam::tt_compute_matBinOp(
                k,
                tt_meta,
                tt_meta,
                a_tt_meta,
                tt::daisy::MatBinOp::ADD
            );

            tt::daisy::foam::copy_ldu_from_device(
                k,
                tt_meta,
                tt_result_diag,
                tt_result_lower,
                tt_result_upper,
                lduAddr()
            );
        }
    #endif

    #if !defined(ENABLE_TT_ADD_ASSIGN) || defined(VERIFY_TT)

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

    #ifdef ENABLE_TT_ADD_ASSIGN
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
            Foam::Info << " this: diag " << had_diag << " lower " << had_lower << " upper " << had_upper << Foam::endl;
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
}


void Foam::lduMatrix::operator-=(const lduMatrix& A)
{
    scalarField * tt_result_diag  = nullptr,
                * tt_result_lower = nullptr,
                * tt_result_upper = nullptr;

    #ifdef ENABLE_TT_SUB_ASSIGN

        if (A.diagPtr_ || A.lowerPtr_ || A.upperPtr_) { // if A is 0, there is nothing to do

            #if (TT_IMPL != TT_IMPL_LDU)
            const bool force_full = true;
            #else
            const bool force_full = false;
            #endif


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
                if (!tt_result_diag) {
                    tt_result_diag = diagPtr_;
                }
                if (!tt_result_lower) {
                    tt_result_lower = lowerPtr_;
                }
                if (!tt_result_upper) {
                    tt_result_upper = upperPtr_;
                }
            #endif
            if (!tt_result_diag && force_full) {
                tt_result_diag = new scalarField(lduAddr().size());
            }
            if (!tt_result_lower && force_full) {
                tt_result_lower = new scalarField(lduAddr().lowerAddr().size());
            }
            if (!tt_result_upper && force_full) {
                tt_result_upper = new scalarField(lduAddr().upperAddr().size());
            }

            auto& k = tt::daisy::foam::require_kernel_launcher();

            auto& tt_meta = tt::daisy::foam::ensure_lduMat_on_device(k, this, is_expand);
            auto& a_tt_meta = tt::daisy::foam::ensure_lduMat_on_device(k, &A);

            tt::daisy::foam::tt_compute_matBinOp(
                k,
                tt_meta,
                tt_meta,
                a_tt_meta,
                tt::daisy::MatBinOp::SUB
            );

            tt::daisy::foam::copy_ldu_from_device(
                k,
                tt_meta,
                tt_result_diag,
                tt_result_lower,
                tt_result_upper,
                lduAddr()
            );
        }
    #endif

    #if !defined(ENABLE_TT_SUB_ASSIGN) || defined(VERIFY_TT)

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

    #ifdef ENABLE_TT_SUB_ASSIGN
    #ifdef VERIFY_TT
        bool fail = false;
        if (tt_result_diag) {
            if (diagPtr_ && !daisy::matches(*tt_result_diag, *diagPtr_)) {
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
            if (lowerPtr_ && !daisy::matches(*tt_result_lower, *lowerPtr_)) {
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
            if (upperPtr_ && !daisy::matches(*tt_result_upper, *upperPtr_)) {
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
}


void Foam::lduMatrix::operator*=(const scalarField& sf)
{
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
}


void Foam::lduMatrix::operator*=(scalar s)
{
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
}


void Foam::lduMatrix::operator/=(const scalarField& sf)
{
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
}


void Foam::lduMatrix::operator/=(scalar s)
{
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
}


// ************************************************************************* //
