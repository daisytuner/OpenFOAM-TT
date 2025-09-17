## 01 - Analysis

### Dataflow

- An experiment creates a `mesh`, which represents the physical region of the experiment in 3D space. Cells are given simple scalar indexes.
- A mesh is represented/decomposed into a set of sub-meshes that are coupled via `interfaces`. Each sub-mesh is represented by a square matrix.
- Most mathematical operations happen on the matrix via different classes, and the results are then synced via the interfaces.
- Matrices represent effects of neighboring cells (i.e. from cell x (row of the matrix) to cell y (column of the matrix)). The diagonal expresses dampening or amplification of a cells content over time, while off-diagonal factors represent relations with neighbors and are hence mostly symmetrical. It expresses relationships between the next-neighbors in the mesh.
- A cells content can have various types. from simple 1D values to 3D-vectors of varying precision.
- Interfaces can select some of the results (cell-contents by index) to send to other matrices to factor into their result and vice-versa.

### ldu Matrix storage native format
- split into non-sparse diagonal vector
- split into vectors of sparse upper and lower triangles. If one is missing, the other triangle is mirrored
- upper and lower triangle non-zero positions are always mirrored, also in the order inside their respective vectors.
- non-zero entries in triangles follow the ordering of the mesh creator (ordered by `faces` = direct cell-neighbor relationship), not some canonical ordering. 
- due to mirroring, each triangle only has a scalar address vector with an "address" for every entry. The location in the matrix in coordinates is defined by using both the upper and lower address as column and row index respectively (i,j just swapped for the other triangle)
=> each sparse entry is coupled with 1 integer value for its column or row index. Sparsity naturally present between cells which are not direct neighbors in any dimension

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

