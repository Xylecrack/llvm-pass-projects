cmake_minimum_required(VERSION 3.13)

project(ControlFlowCheckPass)

find_package(LLVM REQUIRED CONFIG)
include(AddLLVM)
include(LLVMConfig)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_definitions(${LLVM_DEFINITIONS})
#link_directories(${LLVM_LIBRARY_DIRS})
include_directories(${LLVM_INCLUDE_DIRS})

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(runtime)

add_llvm_pass_plugin(ControlFlowCheck
  src/controlflowcheck.cpp
)

