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

#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_VECTOR_SIZE 40
#include <boost/mpl/vector.hpp>

#include <boost/bloom_filter/basic_bloom_filter.hpp>
#include <boost/bloom_filter/counting_bloom_filter.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/bloom_filter/detail/exceptions.hpp>
#include <boost/bloom_filter/hash/murmurhash3.hpp>
#include <boost/cstdint.hpp>

#include <cstdint>
#include <iostream>
#include <mutex>
#include <omp.h>
#include <ompt.h>
#include <stdio.h>
#include <string>
#include <unordered_map>

#define ALWAYS_INLINE __attribute__((always_inline))
#define CALLERPC ((size_t) __builtin_return_address(0))

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
static ompt_get_thread_id_t ompt_get_thread_id;
std::mutex mtx;

// n = 1,000,000, p = 1.0E-10 (1 in 10,000,000,000) → m = 47,925,292 (5.71MB), k = 33
typedef boost::mpl::vector<
		boost::bloom_filters::murmurhash3<size_t, 2>, // 1
		boost::bloom_filters::murmurhash3<size_t, 3>, // 2
		boost::bloom_filters::murmurhash3<size_t, 5>, // 3
		boost::bloom_filters::murmurhash3<size_t, 7>, // 4
		boost::bloom_filters::murmurhash3<size_t, 11>, // 5
		boost::bloom_filters::murmurhash3<size_t, 13>, // 6
		boost::bloom_filters::murmurhash3<size_t, 17>, // 7
		boost::bloom_filters::murmurhash3<size_t, 19>, // 8
		boost::bloom_filters::murmurhash3<size_t, 23>, // 9
		boost::bloom_filters::murmurhash3<size_t, 29>, // 10
		boost::bloom_filters::murmurhash3<size_t, 31>, // 11
		boost::bloom_filters::murmurhash3<size_t, 37>, // 12
		boost::bloom_filters::murmurhash3<size_t, 41>, // 13
		boost::bloom_filters::murmurhash3<size_t, 43>, // 14
		boost::bloom_filters::murmurhash3<size_t, 47>, // 15
		boost::bloom_filters::murmurhash3<size_t, 53>, // 16
		boost::bloom_filters::murmurhash3<size_t, 59>, // 17
		boost::bloom_filters::murmurhash3<size_t, 61>, // 18
		boost::bloom_filters::murmurhash3<size_t, 67>, // 19
		boost::bloom_filters::murmurhash3<size_t, 71>, // 20
		boost::bloom_filters::murmurhash3<size_t, 73>, // 21
		boost::bloom_filters::murmurhash3<size_t, 79>, // 22
		boost::bloom_filters::murmurhash3<size_t, 83>, // 23
		boost::bloom_filters::murmurhash3<size_t, 89>, // 24
		boost::bloom_filters::murmurhash3<size_t, 97>, // 25
		boost::bloom_filters::murmurhash3<size_t, 101>, // 26
		boost::bloom_filters::murmurhash3<size_t, 103>, // 27
		boost::bloom_filters::murmurhash3<size_t, 107>, // 28
		boost::bloom_filters::murmurhash3<size_t, 109>, // 29
		boost::bloom_filters::murmurhash3<size_t, 113>, // 30
		boost::bloom_filters::murmurhash3<size_t, 127>, // 31
		boost::bloom_filters::murmurhash3<size_t, 131>, // 32
		boost::bloom_filters::murmurhash3<size_t, 137>  // 33
> murmurhash3;

class SwordRT {

public:
	SwordRT();
	~SwordRT();

	inline bool Contains(size_t access, AccessType access_type, int tid);
	inline void Insert(size_t access, AccessType access_type, int tid);
	inline void CheckMemoryAccess(size_t access, size_t pc, AccessSize access_size, AccessType access_type, const char *nutex_name = "");
	inline void ReportRace(size_t access, size_t pc, int tid, AccessSize access_size, AccessType access_type, const char *nutex_name = "");
	void clear();

private:
	std::unordered_map<std::string, boost::bloom_filters::counting_bloom_filter<size_t, 100000, 8, murmurhash3>*> filters;
	boost::bloom_filters::basic_bloom_filter<size_t, 1000, murmurhash3> reported_races;

};

#endif  // SWORDRT_RTL_H
