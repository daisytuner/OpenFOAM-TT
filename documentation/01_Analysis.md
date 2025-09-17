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

(excluding stdlib)


### Instrumentation

#### Total

```bash
perf stat -e fp_ops_retired_by_width.pack_128_uops_retired,fp_ops_retired_by_width.pack_256_uops_retired,fp_ops_retired_by_width.pack_512_uops_retired,fp_ops_retired_by_width.scalar_uops_retired -- ./3rdParty/OpenFOAM-dev/bin/foamRun -case ./3rdParty/OpenFOAM-dev/tutorials/incompressibleFluid/cavity
```

#### By Function

```bash
cd build
cmake -DDAISY_INSTRUMENTATION=ON ..
ninja
cd ..

export LD_LIBRARY_PATH=$PWD/build:$LD_LIBRARY_PATH

export FLOP_EVENTS=RETIRED_FP_OPS_BY_WIDTH:SCALAR_UOPS_RETIRED,RETIRED_FP_OPS_BY_WIDTH:PACK128_UOPS_RETIRED,RETIRED_FP_OPS_BY_WIDTH:PACK256_UOPS_RETIRED,RETIRED_FP_OPS_BY_WIDTH:PACK512_UOPS_RETIRED
__DAISY_PAPI_VERSION=0x07020000 __DAISY_INSTRUMENTATION_EVENTS=$FLOP_EVENTS ./3rdParty/OpenFOAM-dev/bin/foamRun -case ./3rdParty/OpenFOAM-dev/tutorials/incompressibleFluid/cavity
```

| Counter                                              |         Value |            FLOP |
| ---------------------------------------------------- | ------------- | --------------- |
| fp_ops_retired_by_width.scalar_uops_retired          | 1,192,414,644 |   1,192,414,644 |
| fp_ops_retired_by_width.pack_128_uops_retired        | 2,948,616,879 |  11,794,467,516 |
| fp_ops_retired_by_width.pack_256_uops_retired        |    47,272,582 |     378,180,656 |
| fp_ops_retired_by_width.pack_512_uops_retired        |    18,889,376 |     302,230,016 |
| Sum                                                  | 4,207,193,481 | 111,270,405,780 |

| Function                                           |                      FLOP |                                             Metric |                     Count |
| -------------------------------------------------- | ------------------------- | -------------------------------------------------- | ------------------------- |
| Foam::GaussSeidelSmoother::smooth                  |             4,849,637,861 |                                                    |                           |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:SCALAR_UOPS_RETIRED        |               236,216,405 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK128_UOPS_RETIRED       |               576,676,082 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK256_UOPS_RETIRED       |                         0 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK512_UOPS_RETIRED       |                       400 |
| Foam::lduMatrix::Amul                              |             3,119,115,198 |                                                    |                           |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:SCALAR_UOPS_RETIRED        |               173,637,830 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK128_UOPS_RETIRED       |               368,184,031 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK256_UOPS_RETIRED       |                         0 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK512_UOPS_RETIRED       |                       160 |
| Foam::lduMatrix::residual                          |               886,261,526 |                                                    |                           |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:SCALAR_UOPS_RETIRED        |                52,070,406 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK128_UOPS_RETIRED       |               104,272,610 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK256_UOPS_RETIRED       |                         0 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK512_UOPS_RETIRED       |                       320 |
| Foam::lduMatrix::negSumDiag                        |               817,803,866 |                                                    |                           |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:SCALAR_UOPS_RETIRED        |                24,320,026 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK128_UOPS_RETIRED       |                99,177,160 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK256_UOPS_RETIRED       |                         0 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK512_UOPS_RETIRED       |                     2,080 |
| Foam::lduMatrix::sumA                              |               548,011,316 |                                                    |                           |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:SCALAR_UOPS_RETIRED        |                18,720,244 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK128_UOPS_RETIRED       |                66,161,384 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK256_UOPS_RETIRED       |                         0 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK512_UOPS_RETIRED       |                         0 |
| Foam::lduMatrix::-=                                |               272,134,605 |                                                    |                           |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:SCALAR_UOPS_RETIRED        |                10,880,005 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK128_UOPS_RETIRED       |                32,655,865 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK256_UOPS_RETIRED       |                         0 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK512_UOPS_RETIRED       |                       240 |
| Foam::lduMatrix::+=                                |               220,837,927 |                                                    |                           |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:SCALAR_UOPS_RETIRED        |                 6,240,007 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK128_UOPS_RETIRED       |                26,823,140 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK256_UOPS_RETIRED       |                         0 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK512_UOPS_RETIRED       |                       400 |
| Foam::lduMatrix::negate                            |               125,593,553 |                                                    |                           |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:SCALAR_UOPS_RETIRED        |                         1 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK128_UOPS_RETIRED       |                15,698,874 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK256_UOPS_RETIRED       |                         0 |
|                                                    |                           | RETIRED_FP_OPS_BY_WIDTH:PACK512_UOPS_RETIRED       |                        80 |