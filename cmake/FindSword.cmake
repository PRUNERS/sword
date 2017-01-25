include(FindPackageHandleStandardArgs)

set(SWORD_LIB LLVMArcher.so)
find_path(SWORD_LIB_PATH
  NAMES LLVMArcher.so
  HINTS ${CMAKE_BINARY_DIR}/lib ${CMAKE_BINARY_DIR}/lib ${LLVM_ROOT}/lib ${CMAKE_INSTALL_PREFIX}/lib
  )
find_path(SWORD_LIB_PATH LLVMArcher.so)

find_package_handle_standard_args(SWORD DEFAULT_MSG SWORD_LIB_PATH)
