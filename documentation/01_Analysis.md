## 01 - Analysis

### Dataflow

- An experiment creates a `mesh`.
- A mesh is represented/decomposed into a set of `matrices`. The matrices are not interdependent and have dependencies called `interfaces`.
- Most mathematical operations happen on the matrix via different classes, and the results are then synced via the interfaces.

### Profiling

```bash
export LD_LIBRARY_PATH=$PWD/build:$LD_LIBRARY_PATH
perf record  --quiet --all-user -e cycles:u -g --call-graph dwarf -o cavity.tmp.profile -- ./3rdParty/OpenFOAM-dev/bin/foamRun -case ./3rdParty/OpenFOAM-dev/tutorials/incompressibleFluid/cavity

perf script -i cavity.tmp.profile -F +srcline --full-source-path > cavitiy.profile
```

| Function                                             | Self-Time |
| ---------------------------------------------------- | --------- |
| Foam::GaussSeidelSmoother:smooth                     |      9.2% |
| Foam::lduMatrix::Amul                                |      3.6% |
| Foam::multiply                                       |      1.6% |
| Foam::fv::gaussGrad::gradf                           |      1.6% |
| Foam::surfaceInterpolationScheme<float>::interpolate |      1.2% |
| Foam::LimitedScheme::calcLimiter                     |      1.1% |
| Foam::GAMGSolver::scale                              |      1.1% |
| Foam::fvc::surfaceIntegrate                          |      1.1% |
| Foam::lduMatrix::residual                            |      1.1% |
| Foam::tmp::dotInterpolate                            |      0.8% |
| Foam::lduMatrix::negSumDiag                          |      0.7% |
| Foam::lduMatrix::solver::normFactor                  |      0.6% |
| Foam::lduMatrix::sumA                                |      0.6% |

(Excluding stdlib functions and constructor)

