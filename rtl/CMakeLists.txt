# find_package(Boost 1.41 COMPONENTS REQUIRED)
# INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
# LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

# add_definitions(-D ZLIB)
add_definitions(-D LZO)
# add_definitions(-D HUFFMAN)
# add_definitions(-D ARITHMETIC)
# add_definitions(-D TCGEN)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3 --std=c++11 -fcxx-exceptions")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O3")

set(SWORD_SOURCES
    sword_rtl.cc
    minilzo.c)

add_library(sword SHARED ${SWORD_SOURCES})
add_library(sword_static STATIC ${SWORD_SOURCES})

set(CMAKE_SHARED_LINKER_FLAGS "-lz -fopenmp")

install(TARGETS sword sword_static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)