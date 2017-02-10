//===-- sword_rtl.h ------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of Sword/Sword, an OpenMP race detector.
//===----------------------------------------------------------------------===//

#ifndef SWORD_RTL_H
#define SWORD_RTL_H

#include "sword_common.h"
#include "minilzo.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#define ALWAYS_INLINE			__attribute__((always_inline))
#define CALLERPC 				((size_t) __builtin_return_address(0))
#define SWORD_DATA 				"sword_data"
#define NUM_OF_ACCESSES			1000000
#define BLOCK_SIZE 				NUM_OF_ACCESSES * sizeof(TraceItem)
#define MB_LIMIT 				BLOCK_SIZE
#define OUT_LEN     			(BLOCK_SIZE + BLOCK_SIZE / 16 + 64 + 3)

#define HEAP_ALLOC(var,size) thread_local lzo_align_t __LZO_MMODEL \
	var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);

// thread_local unsigned char __LZO_MMODEL in  [ BLOCK_SIZE ];
thread_local unsigned char __LZO_MMODEL *out;

#define SWORD_DEBUG 	1
#ifdef SWORD_DEBUG
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
std::atomic<ompt_id_t> current_parallel_idx(0);

// Thread Local Variable
thread_local int tid = 0;
thread_local int __sword_status__ = 0;
thread_local TraceItem __LZO_MMODEL *accesses;
thread_local TraceItem __LZO_MMODEL *accesses1;
thread_local TraceItem __LZO_MMODEL *accesses2;
thread_local uint64_t idx = 0;
thread_local ompt_id_t parallel_idx = 0;
thread_local FILE *datafile = NULL;
thread_local std::future<bool> fut;
thread_local char *buffer = NULL;
thread_local size_t offset = 0;

#endif  // SWORD_RTL_H
