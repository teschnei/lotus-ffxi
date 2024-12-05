# lotus-engine
This repository is the implementation of FFXI using lotus-engine

Currently it can only load maps, non-player character models, animations, some schedulers/generators/particles, and collision meshes.
It is mostly just a demonstration of lotus-engine as I implement various rendering techniques for fun.

# Cloning
The repo uses submodules, so don't forget --recursive when cloning.  If you did forget, you can use `git submodule update --init --recursive` to fix it.

# Build Requirements
* (Windows) Visual Studio 2022
* (Linux) GCC14, Clang18
* Vulkan SDK 1.3.216 or higher
* SDL2 (included in Vulkan SDK on Windows)
* glm (included in Vulkan SDK on Windows)
* Note: I have no idea what version specifically is needed anymore for compilers.  C++ modules support evolves quickly.  At the time of this writing, I use Clang18

# Build
    mkdir build
    cd build
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
    ninja

# Running Requirements
* GPU compatible with VK_KHR_raytracing (RTX2000+, AMD6000+)
* FFXI installed somewhere (located via registry (Windows) or environment variable FFXI_PATH (Linux))

# Run
    cd build/bin
    FFXI_PATH=(...) ./ffxi
