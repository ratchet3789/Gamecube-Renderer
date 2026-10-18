cmake_minimum_required(VERSION 3.18)
project(GCCubeEngine C CXX)

set(CMAKE_EXECUTABLE_SUFFIX ".elf")

# Hardcode Windows forward slashes for devkitPro
set(DEVKITPRO "C:/devkitPro")
set(LIBOGC_INC "${DEVKITPRO}/libogc/include")
set(LIBOGC_LIB "${DEVKITPRO}/libogc/lib/cube")

# PAth to Wiimms ISO Tool Exe
set(WIT_EXEC "${CMAKE_CURRENT_SOURCE_DIR}/Tools/wit-v3.05a-r8638-cygwin64/bin/wit.exe")

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

add_executable(GCCubeEngine ${SOURCES})

# Include libogc headers explicitly (Use BEFORE/PRIVATE so CLion indexer prioritizes it)
target_include_directories(GCCubeEngine PRIVATE
        "${LIBOGC_INC}"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_directories(GCCubeEngine PRIVATE "${LIBOGC_LIB}")
target_link_libraries(GCCubeEngine PRIVATE ogc sysbase m)

# Directories for staging the ISO layout
set(ISO_ROOT "${CMAKE_BINARY_DIR}/iso_root")
set(DOL_OUTPUT "${CMAKE_BINARY_DIR}/GCCubeEngine.dol")
set(ISO_OUTPUT "${CMAKE_BINARY_DIR}/GCCubeEngine.iso")

# Post-build step: Convert ELF to .dol
add_custom_command(
        TARGET GCCubeEngine POST_BUILD
        COMMAND "${DEVKITPRO}/tools/bin/elf2dol.exe" $<TARGET_FILE:GCCubeEngine> "${CMAKE_BINARY_DIR}/GCCubeEngine.dol"
        COMMENT "Converting ELF to GameCube .dol..."
)

if(EXISTS "${ISO_OUTPUT}")
    message(STATUS "Found old ISO, removing it...")
    file(REMOVE "${ISO_OUTPUT}")
endif()

# Post-build step: Assemble ISO directory structure & invoke WIT
add_custom_command(
        TARGET GCCubeEngine POST_BUILD
        # Clean previous staging area
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${ISO_ROOT}"

        # Create sys/ and files/ folders
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ISO_ROOT}/files"

        # Copy your compiled executable as the boot target inside root
        COMMAND ${CMAKE_COMMAND} -E copy "${DOL_OUTPUT}" "${ISO_ROOT}/main.dol"

        # Copy GameCube boot header files into sys/
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/out/sys" "${ISO_ROOT}/sys"

        # Copy executable to sys/main.dol
        COMMAND ${CMAKE_COMMAND} -E copy "${DOL_OUTPUT}" "${ISO_ROOT}/sys/main.dol"

        # Copy raw game assets from /src/Data into /files/Data
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/src/Data" "${ISO_ROOT}/files/Data"

        # Build final ISO image using WIT
        COMMAND "${WIT_EXEC}" COPY "${ISO_ROOT}" "${ISO_OUTPUT}" --iso --force --trunc

        COMMENT "Building GameCube ISO package..."
)