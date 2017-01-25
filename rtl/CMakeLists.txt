cmake_minimum_required(VERSION 3.5)

#find_package(CUDA QUIET REQUIRED)

# Specify CUDA options
#set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}; -std=c++11 -O3 -gencode arch=compute_20,code=sm_20)
#include_directories(/usr/local/cuda-8.0/include) 

#CUDA_ADD_LIBRARY(sword-rt-cuda-static STATIC
#    swordrt_cuda.cu
#)

# target_link_libraries(sword-rt_tsan_strong -Wl,--whole-archive -fPIC ${CMAKE_BINARY_DIR}/lib/clang/4.0.0/lib/linux/libclang_rt.tsan-x86_64.a /home/simone/usr/lib/clang/4.0.0/lib/linux/libclang_rt.tsan-x86_64.a  -Wl,--no-whole-archive)

#install (TARGETS sword-rt-cuda-static
#  ARCHIVE DESTINATION lib
#  LIBRARY DESTINATION lib
#  RUNTIME DESTINATION bin)
