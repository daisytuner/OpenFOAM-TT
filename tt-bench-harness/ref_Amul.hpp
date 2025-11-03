#include <Field.H>
#include <lduMatrix.H>

void refAmul(
    const Foam::lduMatrix& lduMat,
    Foam::scalarField& Apsi,
    const Foam::tmp<Foam::scalarField>& tpsi
);