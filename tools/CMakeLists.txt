add_executable(sword-race-analysis sword-race-analysis.cc ../rtl/minilzo.c)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
set_target_properties(sword-race-analysis PROPERTIES LINK_FLAGS "-lboost_system -lboost_filesystem -pthread")

configure_file(clang-sword.in clang-sword)
configure_file(clang-sword++.in clang-sword++)

install(TARGETS sword-race-analysis RUNTIME DESTINATION bin)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/clang-sword ${CMAKE_CURRENT_BINARY_DIR}/clang-sword++ DESTINATION bin)
