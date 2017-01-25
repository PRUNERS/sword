//===-- swordrt_rtl.h ------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of Sword/SwordRT, an OpenMP race detector.
//===----------------------------------------------------------------------===//

#ifndef SWORDRT_RTL_H
#define SWORDRT_RTL_H

#include <aio.h>
#include <omp.h>
#include <ompt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cstdint>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "minilzo.h"

enum ValueType {
	data = 0,
	parallel_region,
	barrier,
	mutex,
	task
};

enum AccessSize {
	size1 = 0,
	size2,
	size4,
	size8,
	size16
};

enum AccessType {
	unsafe_read,
	unsafe_write,
	atomic_read,
	atomic_write,
};

struct __attribute__ ((__packed__)) AccessInfo {
private:
	uint8_t type;
	uint8_t size_type; // size in first 4 bits, type in last 4 bits
	size_t address;
	size_t pc;

public:
	AccessInfo() {
		type = 0;
		address = 0;
		size_type = 0;
		pc = 0;
	}

	AccessInfo(ValueType t, size_t a, AccessSize as, AccessType at, size_t p) {
		type = t;
		address = a;
		size_type = (as << 4);
		size_type |= at;
		pc = p;
	}

	void setData(ValueType t, size_t a, AccessSize as, AccessType at, size_t p) {
		type = t;
		address = a;
		size_type = (as << 4);
		size_type |= at;
		pc = p;
	}

	ValueType getType() const {
		return (ValueType) type;
	}

	size_t getAddress() const {
		return address;
	}

	AccessSize getAccessSize() const {
		return (AccessSize) (size_type >> 4);
	}

	AccessType getAccessType() const {
		return (AccessType) (size_type & 0x0F);
	}

	size_t getPC() const {
		return pc;
	}
};

#define ALWAYS_INLINE			__attribute__((always_inline))
#define CALLERPC 				((size_t) __builtin_return_address(0))
#define SWORD_DATA 			"SWORD_data"
#define NUM_OF_ACCESSES			1000000
#define BLOCK_SIZE 				NUM_OF_ACCESSES * sizeof(AccessInfo)
#define MB_LIMIT 				BLOCK_SIZE
#define OUT_LEN     			(BLOCK_SIZE + BLOCK_SIZE / 16 + 64 + 3)
#define THREAD_NUM				24

#define HEAP_ALLOC(var,size) thread_local lzo_align_t __LZO_MMODEL \
	var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);

// thread_local unsigned char __LZO_MMODEL in  [ BLOCK_SIZE ];
thread_local unsigned char __LZO_MMODEL *out;

#define SWORDRT_DEBUG 	1
#ifdef SWORDRT_DEBUG
#define ASSERT(x) assert(x);
#define DEBUG(stream, x) 										\
		do {													\
			std::unique_lock<std::mutex> lock(pmtx);			\
			stream << "DEBUG INFO[" << x << "][" << 			\
			__FUNCTION__ << ":" << __FILE__ << ":" << 			\
			std::dec << __LINE__ << "]" << std::endl;			\
		} while(0)
#define INFO(stream, x) 										\
		do {													\
			std::unique_lock<std::mutex> lock(pmtx);			\
			stream << x << std::endl;							\
		} while(0)
#else
#define ASSERT(x)
#define DEBUG(stream, x)
#endif

// Global Variable
std::mutex pmtx;
std::mutex smtx;

// Thread Local Variable
thread_local int __swordomp_status__ = 0;
thread_local AccessInfo __LZO_MMODEL *accesses_heap;
thread_local AccessInfo __LZO_MMODEL *accesses_heap1;
thread_local AccessInfo __LZO_MMODEL *accesses_heap2;
thread_local uint64_t idxs_heap = 0;
thread_local FILE *datafile = NULL;
thread_local std::future<bool> fut;
thread_local char *buffer = NULL;
thread_local size_t offset = 0;

#endif  // SWORDRT_RTL_H
