cmake_minimum_required(VERSION 3.17)
project(HGS_CVRP)

set(CMAKE_CXX_STANDARD 20)


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