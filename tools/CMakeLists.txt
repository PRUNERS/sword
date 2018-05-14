add_definitions(-D_FILE_OFFSET_BITS=64)
# add_definitions(-D PRINT) # PRINT TREES

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3")

set(RBTREE_SOURCES interval_tree.cc rbtree.c)
add_library(rbtree STATIC ${RBTREE_SOURCES})

add_executable(sword-race-analysis sword-race-analysis.cc ${SRCS})
target_link_libraries(sword-race-analysis ${CMAKE_CURRENT_BINARY_DIR}/librbtree.a)
target_link_libraries(sword-race-analysis "-lboost_system -lboost_filesystem -pthread ${GLPK_LIBRARIES}")

add_executable(sword-print-report sword-print-report.cc)
target_link_libraries(sword-print-report "-lboost_system -lboost_filesystem")

configure_file(clang-sword.in clang-sword)
configure_file(clang-sword++.in clang-sword++)
configure_file(sword-offline-analysis.py.in sword-offline-analysis)

install(TARGETS sword-race-analysis sword-print-report RUNTIME DESTINATION bin)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/clang-sword ${CMAKE_CURRENT_BINARY_DIR}/clang-sword++ ${CMAKE_CURRENT_BINARY_DIR}/sword-offline-analysis DESTINATION bin)
