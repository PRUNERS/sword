#
# Copyright (c) 2015-2017, Lawrence Livermore National Security, LLC.
#
# Produced at the Lawrence Livermore National Laboratory
#
# Written by Simone Atzeni (simone@cs.utah.edu), Joachim Protze
# (joachim.protze@tu-dresden.de), Jonas Hahnfeld
# (hahnfeld@itc.rwth-aachen.de), Ganesh Gopalakrishnan, Zvonimir
# Rakamaric, Dong H. Ahn, Gregory L. Lee, Ignacio Laguna, and Martin
# Schulz.
#
# LLNL-CODE-727057
#
# All rights reserved.
#
# This file is part of Sword. For details, see
# https://pruners.github.io/sword. Please also read
# https://github.com/PRUNERS/sword/blob/master/LICENSE.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#    Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the disclaimer below.
#
#    Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the disclaimer (as noted below)
#    in the documentation and/or other materials provided with the
#    distribution.
#
#    Neither the name of the LLNS/LLNL nor the names of its contributors
#    may be used to endorse or promote products derived from this
#    software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE
# LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

cmake_minimum_required(VERSION 3.4.3)

if(POLICY CMP0057)
  cmake_policy(SET CMP0057 NEW)
endif()

set(COMPRESSION "LZO" CACHE STRING "Set the compression algorithm to use.")

if(${COMPRESSION} STREQUAL "LZO")
  add_definitions(-D LZO)
  set(SRCS ${CMAKE_CURRENT_SOURCE_DIR}/rtl/lzo/minilzo.c)
elseif(${COMPRESSION} STREQUAL "SNAPPY")
  add_definitions(-D SNAPPY)
  set(SRCS ${CMAKE_CURRENT_SOURCE_DIR}/rtl/snappy/snappy.cc ${CMAKE_CURRENT_SOURCE_DIR}/rtl/snappy-sinksource.cc)
elseif(${COMPRESSION} STREQUAL "LZ4")
  add_definitions(-D LZ4)
  set(SRCS ${CMAKE_CURRENT_SOURCE_DIR}/rtl/lz4/lz4.c)
else()
  add_definitions(-D LZO)
  set(SRCS ${CMAKE_CURRENT_SOURCE_DIR}/rtl/lzo/minilzo.c)
  # add_definitions(-D HUFFMAN)
  # add_definitions(-D ARITHMETIC)
  # add_definitions(-D TCGEN)
endif()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

# Add cmake directory to search for custom cmake functions
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# Standalone build or part of LLVM?
set(LIBSWORD_STANDALONE_BUILD FALSE)
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    project(sword C CXX)

  set(LIBSWORD_STANDALONE_BUILD TRUE)
endif()

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
  # require at least clang 3.9
  if (CMAKE_C_COMPILER_VERSION VERSION_LESS 3.9)
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

# Check is OSX or Linux (for lib extension)
if(${APPLE})
  set(EXT .dylib)
elseif(${UNIX})
  set(EXT .so)
endif()

if(${LIBSWORD_STANDALONE_BUILD})
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

  # Group test settings.
  set(LIBSWORD_TEST_C_COMPILER ${CMAKE_C_COMPILER} CACHE STRING
    "C compiler to use for testing Sword runtime libraries.")
  set(LIBSWORD_TEST_CXX_COMPILER ${CMAKE_CXX_COMPILER} CACHE STRING
    "C++ compiler to use for testing Sword runtime libraries.")
  set(LIBSWORD_LLVM_TOOLS_DIR "" CACHE PATH "Path to LLVM tools for testing.")

  add_definitions(${LLVM_DEFINITIONS})
  include_directories(${LLVM_INCLUDE_DIRS})
else()
  set(LLVM_ROOT ${CMAKE_INSTALL_PREFIX} CACHE PATH "Root of LLVM install.")
endif()

# Look for OpenMP runtime
if(${LIBSWORD_STANDALONE_BUILD})
  set(OMP_PREFIX /usr/local CACHE PATH "Root of OpenMP runtime install")
  if(NOT EXISTS ${OMP_PREFIX}/include/omp.h)
    message(FATAL_ERROR "OMP_PREFIX (${OMP_PREFIX}) is not a valid OpenMP runtime install")
  endif()
else()
  set(OMP_PREFIX ${LLVM_ROOT} CACHE PATH "Root of OpenMP runtime install")
endif()

find_package(Boost 1.58.0 COMPONENTS system filesystem REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIR})
  link_directories(${Boost_LIBRARY_DIRS})
endif()

find_package(GLPK 1.63 REQUIRED)
if(Boost_FOUND)
  include_directories(${GLPK_INCLUDE_DIRS})
  link_directories(${GLPK_LIBRARIES})
endif()

find_package(Omp)
include_directories(${OMP_INCLUDE_PATH})
link_directories(${OMP_LIB_PATH})
if(NOT ${LLVM_NATIVE_ARCH} STREQUAL "PowerPC")
  find_package(Ompt)
  include_directories(${OMPT_INCLUDE_PATH})
endif()

include(config-ix)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  has_ompt_support("${OMP_LIB_PATH}" "libomp.so" "ompt_start_tool" LIBSWORD_OMPT_SUPPORT)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  has_ompt_support("${OMP_LIB_PATH}" "libomp.dylib" "_ompt_callbacks" LIBSWORD_OMPT_SUPPORT)
endif()

# Set up testing infrastructure.
include(SwordTesting)

set(LIBSWORD_TEST_FLAGS "" CACHE STRING
  "Extra compiler flags to send to the test compiler.")
set(LIBSWORD_TEST_SWORD_FLAGS ${LIBSWORD_TEST_COMPILER_SWORD_FLAGS} CACHE STRING
  "Sword compiler flag to use for testing Sword runtime libraries.")

# Setting directory names

set(LIBSWORD_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(LIBSWORD_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(LIBSWORD_RUNTIME_PATH ${CMAKE_BINARY_DIR}/rtl)
set(LIBSWORD_RTL libsword${EXT})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -undefined dynamic_lookup")
endif()

set(LIBSWORD_LIB_PATH ${CMAKE_BINARY_DIR}/lib)
set(LIBSWORD_LIB LLVMSword${EXT})
set(LIBSWORD_TOOLS_DIR ${LIBSWORD_BASE_DIR}/tools)

add_subdirectory(lib)
add_subdirectory(rtl)
add_subdirectory(test)
add_subdirectory(tools)
