set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3 --std=c++11 -fcxx-exceptions -stdlib=libc++")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O3")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

set(SWORD_SOURCES sword_rtl.cc ${SRCS})

add_library(sword MODULE ${SWORD_SOURCES})
add_library(sword_static STATIC ${SWORD_SOURCES})

set(CMAKE_SHARED_LINKER_FLAGS "-fopenmp")

install(TARGETS sword sword_static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
