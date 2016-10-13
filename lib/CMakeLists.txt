cmake_minimum_required(VERSION 2.8)

# project(SwordRT)

set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost 1.41 COMPONENTS REQUIRED)
message(STATUS "Boost Include: ${Boost_INCLUDE_DIR}")
message(STATUS "Boost Libraries: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost Libraries: ${Boost_LIBRARIES}")

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

# Set SWORD-RT version

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 --std=c++11 -fcxx-exceptions -g") # -fsanitize=thread
set(SWORDRT_VERSION_MAJOR 0)
set(SWORDRT_VERSION_MINOR 1)

set(LLVM_NO_RTTI 1)

set(CMAKE_BUILD_TYPE Release)
 
set(TSAN_SOURCES
  # rtl/swordrt_rtl2.cc)
  # rtl/swordrt_rtl3.cc) # fastest
  # rtl/swordrt_rtl4.cc)
  rtl/swordrt_rtl5.cc)
  
#set(MALLOC_SOURCES
#  rtl/swordrt_malloc_hook.cc)

INCLUDE_DIRECTORIES("/home/simone/usr/include")
set (CMAKE_SHARED_LINKER_FLAGS "-L/home/simone/usr/lib -fopenmp") # -fsanitize=thread
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_BINARY_DIR}/projects/openmp/runtime/src
  /home/simone/LLVM/llvm/projects/sword-rt/lib/rtl
  /home/simone/LLVM/llvm/projects/sword-rt/lib/cuckoofilter)
add_library(sword-rt SHARED ${TSAN_SOURCES})
add_library(sword-rt-static STATIC ${TSAN_SOURCES})
# add_library(sword-rt-malloc-hook SHARED ${MALLOC_SOURCES})

install (TARGETS sword-rt sword-rt-static # sword-rt-malloc-hook
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)
