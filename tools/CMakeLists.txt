if(NOT ${SWORDRT_STANDALONE_BUILD})
  file(COPY solve.py solve_milp.py
    DESTINATION ${LLVM_TOOLS_BINARY_DIR})
endif()
install(PROGRAMS solve.py solve_milp.py DESTINATION bin)
