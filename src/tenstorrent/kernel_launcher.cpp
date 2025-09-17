#include "kernel_launcher.hpp"

KernelLauncher& require_kernel_launcher() {
    if (!kernelLauncher) {
        kernelLauncher = new KernelLauncher();
    }
    return *kernelLauncher;
}