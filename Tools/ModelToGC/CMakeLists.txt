# CMakeList.txt : CMake project for ModelToGC, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("ModelToGC")

set(ASSIMP_INCL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/Assimp/include")
if(WIN32)
  set(ASSIMP_LIB "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/Assimp/lib/x64/assimp-vc145-mt.lib")
else()
  set(ASSIMP_LIB "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/Assimp/lib/x64/libassimp.so")
endif()

# Add source to this project's executable.
add_executable (ModelToGC "main.cpp"
        Types.h)

target_include_directories(ModelToGC PRIVATE ${ASSIMP_INCL_DIR})
target_link_libraries(ModelToGC PRIVATE ${ASSIMP_LIB})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
