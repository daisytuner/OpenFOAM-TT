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

#ifdef __DAISY_INSTRUMENTATION
#include <daisy_rtl/daisy_rtl.h>
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

    const scalarField& Lower = const_cast<const lduMatrix&>(*this).lower();
    const scalarField& Upper = const_cast<const lduMatrix&>(*this).upper();
    scalarField& Diag = diag();

    const labelUList& l = lduAddr().lowerAddr();
    const labelUList& u = lduAddr().upperAddr();

    for (label face=0; face<l.size(); face++)
    {
        Diag[l[face]] += Lower[face];
        Diag[u[face]] += Upper[face];
    }

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

    const scalarField& Lower = const_cast<const lduMatrix&>(*this).lower();
    const scalarField& Upper = const_cast<const lduMatrix&>(*this).upper();
    scalarField& Diag = diag();

    const labelUList& l = lduAddr().lowerAddr();
    const labelUList& u = lduAddr().upperAddr();

    for (label face=0; face<l.size(); face++)
    {
        Diag[l[face]] -= Lower[face];
        Diag[u[face]] -= Upper[face];
    }

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

#ifdef __DAISY_INSTRUMENTATION
    __daisy_instrumentation_exit(region_id);
    __daisy_instrumentation_finalize(region_id);
#endif
}


// ************************************************************************* //
