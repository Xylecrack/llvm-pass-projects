cmake_minimum_required(VERSION 3.13)

project(SCEVArrayAccessPass)

find_package(LLVM REQUIRED CONFIG)
include(AddLLVM)
include(LLVMConfig)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_definitions(${LLVM_DEFINITIONS})
link_directories(${LLVM_LIBRARY_DIRS})
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/include)

add_llvm_pass_plugin(SCEVArrayAccess
  src/arrayaccess.cpp
)
