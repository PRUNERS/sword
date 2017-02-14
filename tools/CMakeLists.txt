add_executable(sword-race-analysis sword-race-analysis.cc)

configure_file(clang-sword.in clang-sword)
configure_file(clang-sword++.in clang-sword++)

install(TARGETS sword-race-analysis RUNTIME DESTINATION bin)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/clang-sword ${CMAKE_CURRENT_BINARY_DIR}/clang-sword++ DESTINATION bin)
