//===-- swordrt_rtl.h ----------------------------------------------*- C++ -*-===//
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

#include "bloom_filter.hpp"
#include <iostream>
#include <unordered_map>
#include <omp.h>
#include <ompt.h>
#include <cstdint>
#include <stdio.h>
#include <string>
# include "count_min_sketch.hpp"

#define ALWAYS_INLINE __attribute__((always_inline))
#define CALLERPC ((uint64_t) __builtin_return_address(0))

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

const char *FilterType[] = {
		"none",
		"unsafe_read",
		"unsafe_write",
		"atomic_read",
		"atomic_write",
		"mutex_read",
		"mutex_write",
		"nutex_read",
		"nutex_write"
};

// Filters Type
#define UNSAFE_READ		"unsafe_read"
#define UNSAFE_WRITE	"unsafe_write"
#define MUTEX_READ      "mutex_read"
#define MUTEX_WRITE     "mutex_write"
#define ATOMIC_READ     "atomic_read"
#define ATOMIC_WRITE    "atomic_write"
// More parallels r/w use parallel id?
// Nutex: use nutex name

thread_local int __swordomp_status__ = 0;
thread_local bool __swordomp_is_critical__ = false;
// thread_local bloom_filter *tls_filter;

class SwordRT {

public:
	SwordRT();
	~SwordRT();

	inline bool Contains(uint64_t access, std::string filter_type);
	inline void Insert(uint64_t access, std::string filter_type);
	inline bool ContainsAndInsert(uint64_t access, std::string filter_type);
	inline void CheckMemoryAccess(uint64_t access, uint64_t pc, AccessSize access_size, AccessType access_type, const char *nutex_name = "");
	inline void ReportRace(uint64_t access, uint64_t pc, AccessSize access_size, AccessType access_type, const char *nutex_name = "");
	bloom_parameters getParameters();
	void clear();

private:
	bloom_parameters parameters;
	std::unordered_map<std::string, bloom_filter*> filters;
	CountMinSketch *c;

	void InitializeBloomFilterParameters();
};

#endif  // SWORDRT_RTL_H
