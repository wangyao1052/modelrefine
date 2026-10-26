cmake_minimum_required(VERSION 3.20)

project(ModelRefine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_DEBUG_POSTFIX "d")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

find_package(OpenCASCADE CONFIG REQUIRED)

add_library(ModelRefine SHARED
    src/modelRefine.cpp
    src/refine.cpp
)

target_compile_definitions(ModelRefine PRIVATE MODELREFINE_LIBRARY)
target_include_directories(ModelRefine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:inc>
)
target_link_libraries(ModelRefine PRIVATE
    TKernel
    TKMath
    TKG2d
    TKG3d
    TKGeomBase
    TKGeomAlgo
    TKBRep
    TKTopAlgo
    TKShHealing
)

# --- tests ---

enable_testing()

add_executable(refine_smoke tests/refine_smoke.cpp)
target_link_libraries(refine_smoke PRIVATE ModelRefine TKPrim TKBO)
add_test(NAME refine_smoke COMMAND refine_smoke)

# --- install ---

include(CMakePackageConfigHelpers)

install(TARGETS ModelRefine EXPORT ModelRefineTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION inc)
install(EXPORT ModelRefineTargets
    NAMESPACE ModelRefine::
    FILE ModelRefineTargets.cmake
    DESTINATION lib/cmake/ModelRefine
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ModelRefineConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ModelRefineConfig.cmake"
    INSTALL_DESTINATION lib/cmake/ModelRefine
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ModelRefineConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ModelRefineConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ModelRefineConfigVersion.cmake"
    DESTINATION lib/cmake/ModelRefine
)
