cmake_minimum_required(VERSION 3.17)

project(ElasticTask)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/tools/cmake/")

# Search for SimGrid
#set(SimGrid_DIR "/home/clem/Code/framagit.org/simgridOfficiel/simgrid/build/" CACHE PATH "Simgrid path")

find_package(SimGrid REQUIRED)
include_directories("${SimGrid_INCLUDE_DIR}" SYSTEM)

MESSAGE("${SimGrid_INCLUDE_DIR}")


option(USE_JAEGERTRACING "Use jaeger traces through opentracing" ON)
configure_file(Elasticity/ElasticConfig.hpp.in ${PROJECT_SOURCE_DIR}/Elasticity/ElasticConfig.hpp)

add_subdirectory(Elasticity)
add_subdirectory(examples)
