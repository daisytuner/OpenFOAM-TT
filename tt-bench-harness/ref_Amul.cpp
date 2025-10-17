#include "ref_Amul.hpp"

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