# true-bitrate

C++ clone of [true-bitrate](https://github.com/dvorapa/true-bitrate)

# Dependencies

- kfr
- glfw3
- glad
- imgui[glfw-binding, opengl3-glad-binding]
- implot

> kfr supports only Clang at the time so it is added manually to the project to better handle the compilation process, all deps but kfr are installed through vcpkg

# Compilation

Built under Windows, code should be portable.

cmdline for Windows, with clang-cl, vcpkg, cmake, MSVC BuildTools installed

```
CALL "PATH_TO\VsDevCmd.bat" -arch=x64
cmake -E env CXXFLAGS="-m64 -fuse-ld=lld" cmake -G"MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE="PATH_TO\vcpkg\scripts\buildsystems\vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows-static -DBUILD_SHARED_LIBS=OFF -DCPU_ARCH=x64 -DCMAKE_BUILD_TYPE=Release -DENABLE_DFT=ON -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_C_COMPILER=clang-cl -B build .
cmake --build build --config Release
```