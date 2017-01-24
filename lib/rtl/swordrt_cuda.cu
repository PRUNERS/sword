#include "swordrt_common.h"
#include <stdio.h>

#define CUDA_WARN(XXX) \
    do { if (XXX != cudaSuccess) printf("CUDA Error: %s[%d], at line %d\n", \
    cudaGetErrorString(XXX), XXX, __LINE__); cudaDeviceSynchronize(); } while (0)


AccessInfo *host_accesses[THREAD_NUM];
AccessInfo *device_accesses;
cudaStream_t stream[THREAD_NUM];

extern thread_local int tid;

void malloc_host() {
	CUDA_WARN(cudaMallocHost((void**)&host_accesses[tid], sizeof(AccessInfo) * NUM_OF_ACCESSES));
}

void malloc_device() {
	// printf("Bytes: %lu\n", sizeof(AccessInfo) * TOTAL_ACCESSES * THREAD_NUM);
	CUDA_WARN(cudaMalloc((void**) &device_accesses, sizeof(AccessInfo) * TOTAL_ACCESSES * THREAD_NUM));
	CUDA_WARN(cudaStreamCreate(&stream[tid]));
}

void set_device() {
	CUDA_WARN(cudaSetDevice(3));
}

void copy_to_device(int chunk) {
	// cudaError_t res = cudaMemcpy(device_accesses + ((sizeof(AccessInfo) * NUM_OF_ACCESSES) * chunk) + (tid * TOTAL_ACCESSES),
	//  		   host_accesses[tid], sizeof(AccessInfo) * NUM_OF_ACCESSES, cudaMemcpyHostToDevice);
	uint64_t offset = ((sizeof(AccessInfo) * NUM_OF_ACCESSES) * chunk) + (tid * TOTAL_ACCESSES);
	CUDA_WARN(cudaMemcpy(device_accesses,
			   host_accesses[tid],
			   sizeof(AccessInfo) * NUM_OF_ACCESSES,
			   cudaMemcpyHostToDevice));
			   /*
	CUDA_WARN(cudaMemcpyAsync(device_accesses,
			   host_accesses[tid],
			   sizeof(AccessInfo) * NUM_OF_ACCESSES,
			   cudaMemcpyHostToDevice,
			   stream[tid]));
			   */
}

