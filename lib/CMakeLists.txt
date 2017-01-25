if(${SWORD_STANDALONE_BUILD})
  set(SWORD_HEADER_FILES ${PROJECT_SOURCE_DIR}/include)
else()
  set(SWORD_HEADER_FILES ${PROJECT_SOURCE_DIR}/tools/sword/include)
endif()

include_directories(${SWORD_HEADER_FILES})

set(SWORD_SOURCE_FILES
  Sword.cpp
  Support/RegisterPasses.cpp
  Support/Util.cpp
  Transforms/Instrumentation/InstrumentParallel.cpp
  )

add_llvm_loadable_module(LLVMSword
  ${SWORD_SOURCE_FILES}
  ${SWORD_HEADER_FILES}
  )
