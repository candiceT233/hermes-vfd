cmake_minimum_required(VERSION 3.10)
project(hermes)

#-----------------------------------------------------------------------------
# Define Options
#-----------------------------------------------------------------------------
option(BUILD_SHARED_LIBS "Build shared libraries (.dll/.so) instead of static ones (.lib/.a)" ON)
option(BUILD_MPI_TESTS "Build tests which depend on MPI" ON)
option(BUILD_OpenMP_TESTS "Build tests which depend on OpenMP" ON)
option(HERMES_ENABLE_COVERAGE "Check how well tests cover code" OFF)
option(HERMES_ENABLE_DOXYGEN "Check how well the code is documented" OFF)

option(HERMES_ENABLE_POSIX_ADAPTER "Build the Hermes POSIX adapter." ON)
option(HERMES_ENABLE_STDIO_ADAPTER "Build the Hermes stdio adapter." ON)
option(HERMES_ENABLE_MPIIO_ADAPTER "Build the Hermes MPI-IO adapter." ON)
option(HERMES_ENABLE_PUBSUB_ADAPTER "Build the Hermes pub/sub adapter." OFF)
option(HERMES_ENABLE_KVSTORE "Build the Hermes KVStore adapter." OFF)
option(HERMES_ENABLE_VFD "Build the Hermes HDF5 Virtual File Driver" OFF)
option(HERMES_ENABLE_PYTHON "Build the Hermes Python wrapper" ON)

message("HERMES_ENABLE_PYTHON: ${HERMES_ENABLE_PYTHON}")

#-----------------------------------------------------------------------------
# Compiler Optimization
#-----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("IN DEBUG MODE")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
    add_compile_definitions(HERMES_LOG_VERBOSITY=10)
else()
    message("IN RELEASE MODE")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O3")
    add_compile_definitions(HERMES_LOG_VERBOSITY=1)
endif()
add_compile_options(-march=native -fomit-frame-pointer)

#-----------------------------------------------------------------------------
# Targets built within this project are exported at Install time for use
# by other projects.
#-----------------------------------------------------------------------------
if(NOT HERMES_EXPORTED_TARGETS)
    set(HERMES_EXPORTED_TARGETS "hrun-targets")
endif()

#-----------------------------------------------------------------------------
# Find Packages
#-----------------------------------------------------------------------------

# HermesShm
find_package(HermesShm CONFIG REQUIRED)
message(STATUS "found hermes_shm.h at ${HermesShm_INCLUDE_DIRS}")

# YAML-CPP
find_package(yaml-cpp REQUIRED)
message(STATUS "found yaml-cpp at ${yaml-cpp_DIR}")

# Catch2
find_package(Catch2 3.0.1 REQUIRED)
message(STATUS "found catch2.h at ${Catch2_CXX_INCLUDE_DIRS}")

# MPICH
if(BUILD_MPI_TESTS)
    find_package(MPI REQUIRED COMPONENTS C CXX)
    message(STATUS "found mpi.h at ${MPI_CXX_INCLUDE_DIRS}")
endif()

# OpenMP
if(BUILD_OpenMP_TESTS)
    find_package(OpenMP REQUIRED COMPONENTS C CXX)
    message(STATUS "found omp.h at ${OpenMP_CXX_INCLUDE_DIRS}")
endif()

# Cereal
find_package(cereal REQUIRED)
if(cereal)
    message(STATUS "found cereal")
endif()

# Pkg-Config
find_package(PkgConfig REQUIRED)
if(PkgConfig)
    message(STATUS "found pkg config")
endif()

# Zeromq
#pkg_check_modules(ZMQ REQUIRED libzmq)
#include_directories(${ZMQ_INCLUDE_DIRS})
#message("Found libzmq at: ${ZMQ_INCLUDE_DIRS}")

# Thallium
find_package(thallium CONFIG REQUIRED)
if(thallium_FOUND)
    message(STATUS "found thallium at ${thallium_DIR}")
endif()

# Boost
# find_package(Boost REQUIRED COMPONENTS regex system filesystem fiber REQUIRED)
find_package(Boost REQUIRED COMPONENTS fiber REQUIRED)
if (Boost_FOUND)
    message(STATUS "found boost at ${Boost_INCLUDE_DIRS}")
endif()
include_directories(${Boost_INCLUDE_DIRS})
message("Boost: ${Boost_LIBRARIES}")

#------------------------------------------------------------------------------
# Setup CMake Environment
#------------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
        ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all Executables.")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
        ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all Libraries")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
        ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all static libraries.")

#------------------------------------------------------------------------------
# Setup install and output Directories
#------------------------------------------------------------------------------
if(NOT HERMES_INSTALL_BIN_DIR)
    set(HERMES_INSTALL_BIN_DIR ${CMAKE_INSTALL_PREFIX}/bin)
endif()
if(NOT HERMES_INSTALL_LIB_DIR)
    set(HERMES_INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif()
if(NOT HERMES_INSTALL_INCLUDE_DIR)
    set(HERMES_INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include)
endif()
if(NOT HERMES_INSTALL_DATA_DIR)
    set(HERMES_INSTALL_DATA_DIR ${CMAKE_INSTALL_PREFIX}/share)
endif()

#-----------------------------------------------------------------------------
# Build Hermes Main Packages
#-----------------------------------------------------------------------------
# Main includes
include_directories(${HermesShm_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
# Required Task includes
include_directories(${CMAKE_SOURCE_DIR}/hrun/include)
include_directories(${CMAKE_SOURCE_DIR}/hrun/tasks_required)
include_directories(${CMAKE_SOURCE_DIR}/hrun/tasks_required/hrun_admin/include)
include_directories(${CMAKE_SOURCE_DIR}/hrun/tasks_required/small_message/include)
include_directories(${CMAKE_SOURCE_DIR}/hrun/tasks_required/remote_queue/include)
include_directories(${CMAKE_SOURCE_DIR}/hrun/tasks_required/worch_proc_round_robin/include)
include_directories(${CMAKE_SOURCE_DIR}/hrun/tasks_required/worch_queue_round_robin/include)
include_directories(${CMAKE_SOURCE_DIR}/hrun/tasks_required/proc_queue/include)
# Task includes
include_directories(${CMAKE_SOURCE_DIR}/tasks)
include_directories(${CMAKE_SOURCE_DIR}/tasks/hermes/include)
include_directories(${CMAKE_SOURCE_DIR}/tasks/bdev/include)
include_directories(${CMAKE_SOURCE_DIR}/tasks/ram_bdev/include)
include_directories(${CMAKE_SOURCE_DIR}/tasks/posix_bdev/include)
include_directories(${CMAKE_SOURCE_DIR}/tasks/hermes_mdm/include)
include_directories(${CMAKE_SOURCE_DIR}/tasks/hermes_blob_mdm/include)
include_directories(${CMAKE_SOURCE_DIR}/tasks/hermes_bucket_mdm/include)
include_directories(${CMAKE_SOURCE_DIR}/tasks/hermes_adapters/include)
include_directories(${CMAKE_SOURCE_DIR}/tasks/data_stager/include)
include_directories(${CMAKE_SOURCE_DIR}/tasks/hermes_data_op/include)
# Test includes
include_directories(${CMAKE_SOURCE_DIR}/test/unit)

set(Hermes_CLIENT_LIBRARIES
        ${HermesShm_LIBRARIES}
        yaml-cpp
        cereal::cereal
        -ldl -lrt -lc -pthread hrun_client)
set(Hermes_CLIENT_DEPS
        hrun_client)
set(Hermes_RUNTIME_LIBRARIES
        ${Hermes_CLIENT_LIBRARIES}
        hrun_runtime
        ${Boost_LIBRARIES})
set(Hermes_RUNTIME_DEPS
        hrun_client hrun_runtime)

set(TEST_MAIN ${CMAKE_SOURCE_DIR}/test/unit)
if (HERMES_ENABLE_PYTHON)
add_subdirectory(external/pybind11)
endif()
add_subdirectory(hrun)
add_subdirectory(src)
add_subdirectory(hermes_adapters)
add_subdirectory(tasks)
add_subdirectory(benchmark)
add_subdirectory(wrapper)
add_custom_target(lint COMMAND bash ${CMAKE_SOURCE_DIR}/ci/lint.sh ${CMAKE_SOURCE_DIR})

#-----------------------------------------------------------------------------
# Build + Enable Testing
#-----------------------------------------------------------------------------
# General function used to hook ctest to python test tool lib
function(pytest test_type test_name)
    set(script ${CMAKE_SOURCE_DIR}/scripts/ci/py_hermes_ci/bin/run_test)
#    add_test(NAME ${test_name}
#            COMMAND ${script} ${test_type} ${test_name} ${CMAKE_BINARY_DIR} ${HERMES_USE_ADDRESS_SANITIZER})
endfunction()

enable_testing()
add_subdirectory(test)

#-----------------------------------------------------------------------------
# Install Hermes Headers
#-----------------------------------------------------------------------------
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/CMake/HermesConfig.cmake
        ${PROJECT_BINARY_DIR}/CMakeFiles/HermesConfig.cmake @ONLY
)

install(
        FILES
        ${PROJECT_BINARY_DIR}/CMakeFiles/HermesConfig.cmake
        DESTINATION
        ${CMAKE_INSTALL_PREFIX}/cmake
)

install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})