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
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#define ALWAYS_INLINE			__attribute__((always_inline))
#define CALLERPC 				((size_t) __builtin_return_address(0))
#define ARCHER_DATA 			"archer_data"
#define NUM_OF_CONFLICTS		100
#define THREAD_NUM				24

#define SWORDRT_DEBUG 	1
#ifdef SWORDRT_DEBUG
#define ASSERT(x) assert(x);
/*
#define DATA(stream, x) 										\
		do {													\
			std::unique_lock<std::mutex> lock(pmtx);			\
			stream << x;										\
			stream.flush();										\
		} while(0)
*/
#define DATA(stream, x) 										\
		do {													\
			stream << x;										\
		} while(0)
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
	uint64_t hash;
	AccessSize size;
	AccessType type;
	size_t pc1;
	uint64_t cell1;
	uint64_t cell2;

	AccessInfo() {
		hash = 0;
		size = size4;
		type = none;
		pc1 = 0;
		cell1 = 0;
		cell2 = 0;
	}

	AccessInfo(uint64_t h, AccessSize as, AccessType t, size_t p1, uint64_t c1, uint64_t c2) {
		hash = h;
		size = as;
		type = t;
		pc1 = p1;
		cell1 = c1;
		cell1 = c2;
	}
};

class ByHash
{
public:
	ByHash(uint64_t hash) : hash(hash) {}
    bool operator() (const AccessInfo &access) const { return access.hash == hash; }
private:
    const uint64_t hash;
};

//	bool by_access(const size_t a, const AccessSize s) {
//	    return ((address == a) && (size == s));
//	};
//
//	bool by_pcs(size_t p1, size_t p2) {
//		return ((pc1 == p1) && (pc2 == p2));
//	};

// Global Variable
std::mutex pmtx;
std::mutex smtx;
std::unordered_set<uint64_t> access_tsan_checks;
std::unordered_set<uint64_t> entry_tsan_checks;
bool access_tsan_enabled;
bool entry_tsan_enabled;

// Thread Local Variable
thread_local int __swordomp_status__ = 0;

std::vector<AccessInfo> accesses[THREAD_NUM];
std::ofstream datafile[THREAD_NUM];
uint8_t __swordomp_is_critical__[THREAD_NUM] = { false };
unsigned __swordrt_prev_offset__[THREAD_NUM] = { 0 };
unsigned __swordrt_barrier__[THREAD_NUM] = { 0 };

#endif  // SWORDRT_RTL_H
