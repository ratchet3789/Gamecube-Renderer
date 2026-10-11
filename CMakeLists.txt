cmake_minimum_required(VERSION 3.18)
project(GCCubeEngine C CXX)

set(CMAKE_EXECUTABLE_SUFFIX ".elf")

# Hardcode Windows forward slashes for devkitPro
set(DEVKITPRO "C:/devkitPro")
set(LIBOGC_INC "${DEVKITPRO}/libogc/include")
set(LIBOGC_LIB "${DEVKITPRO}/libogc/lib/cube")

# Ensure Debug mode is active (includes -g or -g3 for PowerPC toolchains like devkitPPC)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose build type" FORCE)
endif()

# Force full debug flags and map generation for GC/Wii ELF targets
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g3 -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -O0 -DDEBUG")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mrvl -mcpu=750 -meabi -mhard-float")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mrvl")

# Generate a .map file alongside your ELF output
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map,${CMAKE_PROJECT_NAME}.map")

file(GLOB_RECURSE SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_executable(GCCubeEngine ${SOURCES}
        src/memory.h
        src/Debug.h
        src/input_manager.h
        src/observer.h
        src/generic_types.h
        src/AssetTypes/StaticMesh.h)

# Include libogc headers explicitly (Use BEFORE/PRIVATE so CLion indexer prioritizes it)
target_include_directories(GCCubeEngine PRIVATE
        "${LIBOGC_INC}"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_directories(GCCubeEngine PRIVATE "${LIBOGC_LIB}")
target_link_libraries(GCCubeEngine PRIVATE ogc sysbase m)

# Post-build step: Convert ELF to .dol
add_custom_command(
        TARGET GCCubeEngine POST_BUILD
        COMMAND "${DEVKITPRO}/tools/bin/elf2dol.exe" $<TARGET_FILE:GCCubeEngine> "${CMAKE_BINARY_DIR}/GCCubeEngine.dol"
        COMMENT "Converting ELF to GameCube .dol..."
)