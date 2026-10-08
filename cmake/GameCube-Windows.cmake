set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR powerpc)

# Force forward slashes on all root paths
set(DEVKITPRO "C:/devkitPro" CACHE PATH "devkitPro root" FORCE)
set(DEVKITPPC "C:/devkitPro/devkitPPC" CACHE PATH "devkitPPC root" FORCE)

# Cross-compiler executables (MUST use forward slashes)
set(CMAKE_C_COMPILER "C:/devkitPro/devkitPPC/bin/powerpc-eabi-gcc.exe" CACHE FILEPATH "C Compiler" FORCE)
set(CMAKE_CXX_COMPILER "C:/devkitPro/devkitPPC/bin/powerpc-eabi-g++.exe" CACHE FILEPATH "C++ Compiler" FORCE)
set(CMAKE_AR "C:/devkitPro/devkitPPC/bin/powerpc-eabi-ar.exe" CACHE FILEPATH "Archiver" FORCE)
set(CMAKE_RANLIB "C:/devkitPro/devkitPPC/bin/powerpc-eabi-ranlib.exe" CACHE FILEPATH "Ranlib" FORCE)
set(CMAKE_NINJA_FORCE_RESPONSE_FILE 1)

# Bypass compiler testing checks
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)

# Hardware Architecture Flags
set(ARCH_FLAGS "-mrvl -mcpu=750 -meabi -mhard-float")
set(CMAKE_C_FLAGS_INIT "${ARCH_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_INIT "${ARCH_FLAGS}" CACHE STRING "" FORCE)

# Target Search Paths
set(CMAKE_FIND_ROOT_PATH "C:/devkitPro/devkitPPC" "C:/devkitPro/libogc")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)