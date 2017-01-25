file(COPY clang-sword clang-sword++
  DESTINATION ${LLVM_TOOLS_BINARY_DIR})
install(PROGRAMS clang-sword clang-sword++ DESTINATION bin)
