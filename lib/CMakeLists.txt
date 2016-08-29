cmake_minimum_required(VERSION 2.8)

# project(SwordRT)

# Set SWORD-RT version
set(SWORDRT_VERSION_MAJOR 0)
set(SWORDRT_VERSION_MINOR 1)

set(LLVM_NO_RTTI 1)

set(CMAKE_BUILD_TYPE Release)

set(TSAN_SOURCES
  rtl/swordrt_rtl.cc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(sword-rt SHARED ${TSAN_SOURCES})

install (TARGETS sword-rt
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)
