add_executable(sword-race-analysis sword-race-analysis.cc ../rtl/minilzo.c)
add_executable(sword-print-report sword-print-report.cc)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3 -stdlib=libstdc++")
set_target_properties(sword-race-analysis sword-print-report PROPERTIES LINK_FLAGS "-lboost_system -lboost_filesystem -pthread")

configure_file(clang-sword.in clang-sword)
configure_file(clang-sword++.in clang-sword++)
configure_file(sword-offline-analysis.py.in sword-offline-analysis)

install(TARGETS sword-race-analysis sword-print-report RUNTIME DESTINATION bin)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/clang-sword ${CMAKE_CURRENT_BINARY_DIR}/clang-sword++ ${CMAKE_CURRENT_BINARY_DIR}/sword-offline-analysis DESTINATION bin)
