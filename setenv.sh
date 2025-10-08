
export WM_PROJECT_DIR=$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/3rdParty/OpenFOAM-dev")
export TT_FOAM_KERNEL_DIR=$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/daisy-tt-rt/src/tenstorrent-kernels")
# libOpenFOAM.so looks for etc in this place to load its config ON STARTUP. So our gtest needs this. And because gtest tries to execute it as part of the build its needed during build as well

#. ${WM_PROJECT_DIR}/etc/custom_bashrc # DO NOT DO THIS. The Env vars interfere with libtt_metal.so and its MPI dependencies.


