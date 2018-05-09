include(FindPackageHandleStandardArgs)

set(LIBSWORD_LIB LLVMSword.so)
find_path(LIBSWORD_LIB_PATH
  NAMES LLVMSword.so
  HINTS ${CMAKE_BINARY_DIR}/lib ${CMAKE_BINARY_DIR}/lib ${LLVM_ROOT}/lib ${CMAKE_INSTALL_PREFIX}/lib
  )
find_path(LIBSWORD_LIB_PATH LLVMSword.so)

find_package_handle_standard_args(LIBSWORD DEFAULT_MSG LIBSWORD_LIB_PATH)
