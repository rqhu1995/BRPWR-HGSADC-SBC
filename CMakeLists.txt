cmake_minimum_required(VERSION 3.17)
project(HGS_CVRP)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/.cache/fetchcontent")
file(MAKE_DIRECTORY "${FETCHCONTENT_BASE_DIR}")

# HDF5 C library (system install required: libhdf5-dev on Debian/Ubuntu)
find_package(HDF5 REQUIRED COMPONENTS C)

# HighFive header-only C++ wrapper
set(HIGHFIVE_USE_BOOST OFF CACHE BOOL "" FORCE)
set(HIGHFIVE_USE_EIGEN OFF CACHE BOOL "" FORCE)
set(HIGHFIVE_USE_XTENSOR OFF CACHE BOOL "" FORCE)
set(HIGHFIVE_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(HIGHFIVE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(HIGHFIVE_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
FetchContent_Declare(
    highfive
    GIT_REPOSITORY https://github.com/BlueBrain/HighFive.git
    GIT_TAG v2.10.1
)
FetchContent_MakeAvailable(highfive)

# Assuming the header files are in the same directory as the source files
include_directories(
        "Program"
)

add_executable(
        main Program/main.cpp
        Program/Instance.cpp
        Program/Params.cpp
        Program/Individual.cpp
        Program/LocalSearch.cpp
        Program/Population.cpp
        Program/Genetic.cpp
        Program/helpers/Utils.cpp
        Program/helpers/TimeHelper.cpp
        Program/helpers/Args.cpp
        Program/helpers/cmdparser.hpp
        Program/RepositionScheme.h
        Program/main.cpp
        Program/helpers/alias.h
)

target_link_libraries(main PRIVATE HighFive)
