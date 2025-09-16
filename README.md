# OpenFOAM-TT

This project will build a libOpenFOAM.so that should be drop-in compatible to the original one. But build it with CMake, fast and controlled and can easily swap out classes such as the lduMatrix to experiment with it and create minimal tests for offloading.

To run with this custom libOpenFOAM.so, simply prepend our build path to LD_LIBRARY_PATH before running foamRun as normal.

Even though we included libPstream dummy's contents into this lib instead of building it as well or reusing the openFoam build of it, it works for cavity.