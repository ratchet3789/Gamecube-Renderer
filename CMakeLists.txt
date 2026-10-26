cmake_minimum_required(VERSION 3.18)
project(GameCubeEngine C CXX ASM)

set(CMAKE_EXECUTABLE_SUFFIX ".elf")

# Hardcode Windows directories for devkitPro
set(DEVKITPRO "C:/devkitPro")
set(LIBOGC_INC "${DEVKITPRO}/libogc/include")
set(LIBOGC_LIB "${DEVKITPRO}/libogc/lib/cube")

# Hardcode Game Data Directories
set(TAR_ROOT "${CMAKE_SOURCE_DIR}/src/Data")
set(TAR_OUTPUT "${CMAKE_SOURCE_DIR}/src/GameCubeEngine.tar")

# Helper paths derived from TAR_ROOT
get_filename_component(TAR_PARENT_DIR "${TAR_ROOT}" DIRECTORY)
get_filename_component(TAR_DIR_NAME "${TAR_ROOT}" NAME)
set(ASM_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/GameCubeEngine_tar.s")

# Ensure Debug mode is active
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose build type" FORCE)
endif()

# Force full debug flags and map generation for GC ELF targets
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g3 -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -O0 -DDEBUG")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mrvl -mcpu=750 -meabi -mhard-float")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mrvl")

# Generate a .map file alongside the ELF output
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map,${CMAKE_PROJECT_NAME}.map")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-warn-execstack -Wl,--no-warn-rwx-segments")

# Archive the Data folder using CMake's cross-platform tar tool
add_custom_command(
        OUTPUT ${TAR_OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E tar cf "${TAR_OUTPUT}" --format=gnutar "${TAR_DIR_NAME}"
        WORKING_DIRECTORY "${TAR_PARENT_DIR}"
        COMMENT "Packing ${TAR_DIR_NAME} into ${TAR_OUTPUT}"
)

# Convert the tarball into an assembly source file using bin2s
add_custom_command(
        OUTPUT ${ASM_OUTPUT}
        COMMAND "C:/devkitPro/tools/bin/bin2s.exe" "${TAR_OUTPUT}" > "${ASM_OUTPUT}"
        DEPENDS ${TAR_OUTPUT}
        COMMENT "Converting ${TAR_OUTPUT} to assembly object"
)

file(GLOB_RECURSE SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
				
)

add_executable(GameCubeEngine ${SOURCES}
	    ${ASM_OUTPUT}
)

# Include libogc headers explicitly (Use BEFORE/PRIVATE so CLion indexer prioritizes it)
target_include_directories(GameCubeEngine PRIVATE
        "${LIBOGC_INC}"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_directories(GameCubeEngine PRIVATE "${LIBOGC_LIB}")
target_link_libraries(GameCubeEngine PRIVATE ogc sysbase m)

# Post-build step: Convert ELF to .dol
add_custom_command(
        TARGET GameCubeEngine POST_BUILD
        COMMAND "${DEVKITPRO}/tools/bin/elf2dol.exe" $<TARGET_FILE:GameCubeEngine> "${CMAKE_BINARY_DIR}/GameCubeEngine.dol"
        COMMENT "Converting ELF to GameCube .dol..."
)