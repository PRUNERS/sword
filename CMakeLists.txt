cmake_minimum_required(VERSION 3.4.3)

if(POLICY CMP0057)
  cmake_policy(SET CMP0057 NEW)
endif()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

find_package(Boost 1.58 COMPONENTS REQUIRED)
INCLUDE_DIRECTORIES(${BOOST_INCLUDE_DIR})
LINK_DIRECTORIES(${BOOST_LIBRARY_DIRS})

# Add cmake directory to search for custom cmake functions
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# Standalone build or part of LLVM?
set(SWORD_STANDALONE_BUILD FALSE)
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    project(sword C CXX)

  set(SWORD_STANDALONE_BUILD TRUE)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # require at least clang 3.9
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.9)
        message(FATAL_ERROR "Clang version must be at least 3.9!")
    endif()
else()
    message(FATAL_ERROR "You are using an unsupported compiler! The required compiler is Clang version >= 3.9.")
endif()

string(SUBSTRING ${CMAKE_CXX_COMPILER_VERSION} 0 3 LLVM_VERSION)
string(REPLACE "." "" LLVM_VERSION ${LLVM_VERSION})
add_definitions(-DLLVM_VERSION=${LLVM_VERSION})

# These include files are in the cmake/ subdirectory
include(LibswordUtils)

if(${SWORD_STANDALONE_BUILD})
  # LLVM detection part
  set(LLVM_ROOT "" CACHE PATH "Root of LLVM install.")
  find_package(LLVM MODULE REQUIRED)

  # Sanity check
  if(NOT EXISTS ${LLVM_ROOT}/include/llvm)
    message(FATAL_ERROR "LLVM_ROOT (${LLVM_ROOT}) is not a valid LLVM install")
  endif()

  # Incorporate the CMake features provided by LLVM:
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${LLVM_ROOT}/lib/cmake/llvm")

  include(LLVMConfig)
  include(HandleLLVMOptions)
  include(AddLLVM)

  add_definitions(${LLVM_DEFINITIONS})
  include_directories(${LLVM_INCLUDE_DIRS})
endif()

# Look for OpenMP runtime
if(${SWORD_STANDALONE_BUILD})
  set(OMP_PREFIX /usr/local CACHE PATH "Root of OpenMP runtime install")
  if(NOT EXISTS ${OMP_PREFIX}/include/omp.h)
    message(FATAL_ERROR "OMP_PREFIX (${OMP_PREFIX}) is not a valid OpenMP runtime install")
  endif()
else()
  set(OMP_PREFIX ${LLVM_ROOT} CACHE PATH "Root of OpenMP runtime install")
endif()

# These include files are in the cmake/ subdirectory
include(LibswordUtils)

find_package(Omp)
include_directories(${OMP_INCLUDE_PATH})
find_package(Ompt)
include_directories(${OMPT_INCLUDE_PATH})

add_subdirectory(lib)
add_subdirectory(rtl)

set(SWORD_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SWORD_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(SWORD_RUNTIME_PATH ${CMAKE_BINARY_DIR}/rtl)
set(SWORD_RTL libsword.so)

add_subdirectory(test)
add_subdirectory(tools)
