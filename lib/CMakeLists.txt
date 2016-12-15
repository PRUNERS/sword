set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost 1.41 COMPONENTS REQUIRED)
message(STATUS "Boost Include: ${Boost_INCLUDE_DIR}")
message(STATUS "Boost Libraries: ${Boost_LIBRARY_DIRS}")
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

# Set SWORD-RT version
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 --std=c++11 -fcxx-exceptions -g") # -fsanitize=thread
set(SWORDRT_VERSION_MAJOR 0)
set(SWORDRT_VERSION_MINOR 1)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/rtl
  ${CMAKE_CURRENT_SOURCE_DIR}/sanitizers_interface
  )

set(CMAKE_BUILD_TYPE Release)

set(SWORD_SOURCES
  rtl/swordrt_rtl.cc)

set(SWORD_TSAN_SOURCES
  sanitizers_interface/swordrt_tsan_interface.cc
  sanitizers_interface/swordrt_ompsan_interface.cc)
set_source_files_properties(swordrt_tsan_interface.cc PROPERTIES CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIE")

set(CMAKE_SHARED_LINKER_FLAGS "-fopenmp")

add_library(sword-rt SHARED ${SWORD_SOURCES})
add_library(sword-rt-static STATIC ${SWORD_SOURCES})
add_library(sword-rt_tsan_weak SHARED ${SWORD_TSAN_SOURCES})
add_library(sword-rt_tsan_strong SHARED ${SWORD_TSAN_SOURCES})

# target_link_libraries(sword-rt_tsan_strong -Wl,--whole-archive -fPIC ${CMAKE_BINARY_DIR}/lib/clang/4.0.0/lib/linux/libclang_rt.tsan-x86_64.a /home/simone/usr/lib/clang/4.0.0/lib/linux/libclang_rt.tsan-x86_64.a  -Wl,--no-whole-archive)

install (TARGETS sword-rt sword-rt-static sword-rt_tsan_weak sword-rt_tsan_strong
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)
