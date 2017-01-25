# add_definitions(-D ZLIB)
add_definitions(-D LZO)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 --std=c++11 -fcxx-exceptions -g")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g")

set(SWORD_SOURCES
    sword_rtl.cc
    minilzo.c)
  
add_library(sword SHARED ${SWORD_SOURCES})
add_library(sword_static STATIC ${SWORD_SOURCES})

set(CMAKE_SHARED_LINKER_FLAGS "-lz -fopenmp")

install(TARGETS sword sword_static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
