cmake_minimum_required(VERSION 2.8)

# project(SwordRT)

set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost 1.41 COMPONENTS REQUIRED)
message(STATUS "Boost Include: ${Boost_INCLUDE_DIR}")
message(STATUS "Boost Libraries: ${Boost_LIBRARY_DIRS}")

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

# Set SWORD-RT version

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 --std=c++11 -fcxx-exceptions -g") # -fsanitize=thread
set(SWORDRT_VERSION_MAJOR 0)
set(SWORDRT_VERSION_MINOR 1)

set(LLVM_NO_RTTI 1)

set(CMAKE_BUILD_TYPE Release)

set(TSAN_SOURCES
  rtl/swordrt_rtl.cc)
  # rtl/swordrt_rtl2.cc)

INCLUDE_DIRECTORIES("/home/simone/usr/include")
set(CMAKE_SHARED_LINKER_FLAGS "-L/home/simone/usr/lib -fopenmp /home/simone/usr/lib/clang/4.0.0/lib/linux/libclang_rt.tsan-x86_64.a") # -fsanitize=thread
# set(CMAKE_EXE_LINKER_FLAGS "/home/simone/usr/lib/clang/4.0.0/lib/linux/libclang_rt.tsan-x86_64.a")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_BINARY_DIR}/projects/openmp/runtime/src
  /home/simone/LLVM/llvm/projects/sword-rt/lib/rtl
  /home/simone/LLVM/llvm/projects/compiler-rt/lib/tsan/rtl
  /home/simone/LLVM/llvm/projects/compiler-rt/lib)
add_library(sword-rt SHARED ${TSAN_SOURCES})
add_library(sword-rt-static STATIC ${TSAN_SOURCES})

install (TARGETS sword-rt sword-rt-static
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)
