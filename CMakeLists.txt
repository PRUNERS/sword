cmake_minimum_required(VERSION 3.4.3)

# Standalone build or part of LLVM?
set(SWORDRT_STANDALONE_BUILD FALSE)
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  project(sword-rt)
  set(SWORDRT_STANDALONE_BUILD TRUE)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(${SWORDRT_STANDALONE_BUILD})
  # LLVM detection part
  set(LLVM_ROOT "" CACHE PATH "Root of LLVM install.")
  find_package(LLVM 4.0 MODULE REQUIRED)

  # A bit of a sanity check:
  if( NOT EXISTS ${LLVM_ROOT}/include/llvm )
    message(FATAL_ERROR "LLVM_ROOT (${LLVM_ROOT}) is not a valid LLVM
    install")
  endif()

  # Incorporate the CMake features provided by LLVM:
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${LLVM_ROOT}/lib/cmake/llvm")

  add_definitions(${LLVM_DEFINITIONS})
  include_directories(${LLVM_INCLUDE_DIRS})
endif()

find_package(Omp)
find_package(Ompt)
include_directories(${OMP_INCLUDE_PATH})
include_directories(${OMPT_INCLUDE_PATH})
add_subdirectory(lib)
add_subdirectory(tools)
