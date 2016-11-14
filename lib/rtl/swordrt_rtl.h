//===-- swordrt_rtl.h ------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of Archer/SwordRT, an OpenMP race detector.
//===----------------------------------------------------------------------===//

#ifndef SWORDRT_RTL_H
#define SWORDRT_RTL_H

#include <omp.h>
#include <ompt.h>
#include <stdio.h>
#include <stdlib.h>

#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <unordered_set>
#include <unordered_map>

#define ALWAYS_INLINE			__attribute__((always_inline))
#define CALLERPC 				((size_t) __builtin_return_address(0))
#define ARCHER_DATA 			"archer_data"

#define SWORDRT_DEBUG 	1
#ifdef SWORDRT_DEBUG
#define ASSERT(x) assert(x);
#define DATA(stream, x) 										\
		do {													\
			std::unique_lock<std::mutex> lock(pmtx);			\
			stream << x;										\
			stream.flush();										\
		} while(0)
#define DEBUG(stream, x) 										\
		do {													\
			std::unique_lock<std::mutex> lock(pmtx);			\
			stream << "DEBUG INFO[" << x << "][" << 			\
			__FUNCTION__ << ":" << __FILE__ << ":" << 			\
			std::dec << __LINE__ << "]" << std::endl;			\
		} while(0)
// #define INFO(stream, x)
#define INFO(stream, x) 										\
		do {													\
			std::unique_lock<std::mutex> lock(pmtx);			\
			stream << x << std::endl;	\
		} while(0)
#else
#define ASSERT(x)
#define DEBUG(stream, x)
#endif

enum AccessSize {
	size1 = 0,
	size2,
	size4,
	size8,
	size16
};

enum AccessType {
	none = 0,
	unsafe_read,
	unsafe_write,
	atomic_read,
	atomic_write,
	mutex_read,
	mutex_write,
	nutex_read,
	nutex_write
};

struct AccessInfo
{
	size_t address;
	size_t prev_address;
	unsigned stride;
	uint64_t diff;
	uint64_t count;
	AccessSize size;
	AccessType type;
	size_t pc;

	AccessInfo() {
		address = 0;
		prev_address = 0;
		stride = 0;
		diff = 0;
		count = 0;
		size = size4;
		type = none;
		pc = 0;
	}

	AccessInfo(size_t a, size_t pa, unsigned s, uint64_t d,
			uint64_t c, AccessSize as, AccessType t, size_t p) {
		address = a;
		prev_address = pa;
		stride = s;
		diff = d;
		count = c;
		size = as;
		type = t;
		pc = p;
	}
};

// Global Variable
std::mutex pmtx;
std::ofstream datafile;
std::ofstream entrydatafile;
std::ofstream accessdatafile;
void *handle;
uint64_t swordrt_accesses_min = UINTMAX_MAX;
uint64_t swordrt_accesses_max = 0;
uint64_t swordrt_accesses_count;
uint64_t swordrt_function_min = UINTMAX_MAX;
uint64_t swordrt_function_max = 0;
uint64_t swordrt_function_count;
static ompt_get_thread_id_t ompt_get_thread_id;
static ompt_get_parallel_id_t ompt_get_parallel_id;
std::mutex smtx;
std::unordered_set<uint64_t/* , Hasher */> access_tsan_checks;
std::unordered_set<uint64_t/* , Hasher */> entry_tsan_checks;
bool access_tsan_enabled;
bool entry_tsan_enabled;

// Thread Local Variable
thread_local uint64_t tid = 0;
thread_local size_t *stack;
thread_local size_t stacksize;
thread_local int __swordomp_status__ = 0;
thread_local uint8_t __swordomp_is_critical__ = false;
thread_local std::unordered_map<uint64_t, AccessInfo/* , Hasher, HasherEqualFn */> accesses;
thread_local std::unordered_set<uint64_t/* , Hasher */> tsan_checks;
thread_local unsigned __swordrt_prev_offset__ = 0;
thread_local unsigned __swordrt_barrier__ = 0;
thread_local uint64_t __swordrt_hash__ = 0;

#endif  // SWORDRT_RTL_H
