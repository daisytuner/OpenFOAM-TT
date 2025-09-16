# OpenFOAM-TT

This project builds the core `libOpenFOAM` of the OpenFOAM project separately as a cmake project.
The project is structured such that original sources are used, except for symbol-equivalent replacements provided by this project.
The result shared library can be used with the original OpenFOAM driver by prepeding this project's build folder to the LD_LIBRARY_PATH.

## libOpenFOAM

The core `libOpenFOAM` defines the basic abstractions for operations on sparse matrices used by the different abstract solvers.
The structure of the project allows to explore different implementations of these matrices with a minimal setup and simple testing framework.
In particular, we explore different offloading strategies with minimal impact on the users of the matrices.

## Usage

### Dependencies

```bash
sudo apt-get install -y build-essential libopenmpi-dev zlib1g-dev gnuplot gnuplot-x11 libxt-dev cmake flex ninja-build
sudo apt-get install -y libxml2-dev libhdf5-dev libavfilter-dev libtheora-dev libgl2ps-dev libx11-dev libqt5x11extras5-dev libglew-dev libutfcpp-dev 
```

### Build

First, generate the original include paths of OpenFOAM and build the project.

```bash
cd 3rdParty/OpenFOAM-dev/

export WM_PROJECT_DIR=`pwd`
. ./etc/custom_bashrc

./wmake/wmakeLnIncludeAll
./Allwmake -j$(nproc)

cd ../../
```

Second, to build this project simply run:

```bash
mkdir build && cd build

cmake -G Ninja ..
ninja
```

### Run (Cavity)

Generate the additional files once:

```bash
cd ${WM_PROJECT_DIR}/tutorials/incompressibleFluid/cavity
${FOAM_APPBIN}/blockMesh
cd ${WM_PROJECT_DIR}
cd ../../
```

Run the experiment:

```bash
LD_LIBRARY_PATH=$PWD/build:$LD_LIBRARY_PATH ./3rdParty/OpenFOAM-dev/bin/foamRun -case ./3rdParty/OpenFOAM-dev/tutorials/incompressibleFluid/cavity
```
