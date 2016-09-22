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
#include <boost/bloom_filter/basic_bloom_filter.hpp>
#include <boost/bloom_filter/counting_bloom_filter.hpp>
#include <boost/bloom_filter/detail/exceptions.hpp>
#include <boost/bloom_filter/hash/murmurhash3.hpp>
#include <boost/cstdint.hpp>
#include <boost/mpl/vector.hpp>

#include <omp.h>
#include <ompt.h>
#include <stdio.h>

#include <cstdint>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>

#define SWORDRT_DEBUG 1

std::mutex pmtx;
#ifdef SWORDRT_DEBUG
#define ASSERT(x) assert(x);
#define DEBUG(stream, x) 										\
		do {													\
			std::unique_lock<std::mutex> lock(pmtx);		\
			stream << "DEBUG INFO[" << x << "][" << __FUNCTION__ << ":" << __FILE__ << ":" << std::dec << __LINE__ << "]" << std::endl;	\
		} while(0)
#else
#define ASSERT(x)
#define DEBUG(stream, x)
#endif

#define ALWAYS_INLINE				__attribute__((always_inline))
#define CALLERPC					((size_t) __builtin_return_address(0))

#define NUM_OF_REPORTED_RACES		1000
#define NUM_OF_ITEMS				100000
#define TID_NUM_OF_BITS				8

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

static ompt_get_thread_id_t ompt_get_thread_id;
std::mutex exitMtx;
std::mutex raceMtx;
std::mutex filterMtx;
std::mutex threadMtx;
std::mutex queueMtx;

thread_local uint64_t tid = 0;
thread_local void *stack;
thread_local size_t stacksize;
thread_local int __swordomp_status__ = 0;
thread_local bool __swordomp_is_critical__ = false;
thread_local unsigned count = 0;

// n = 1,000,000, p = 1.0E-10 (1 in 10,000,000,000) → m = 47,925,292 (5.71MB), k = 33
// #define MURMUR3
#ifdef MURMUR3
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
> hash_function;
#else
typedef boost::mpl::vector<
		boost::bloom_filters::boost_hash<size_t, 2>, // 1
		boost::bloom_filters::boost_hash<size_t, 3>, // 2
		boost::bloom_filters::boost_hash<size_t, 5>, // 3
		boost::bloom_filters::boost_hash<size_t, 7>, // 4
		boost::bloom_filters::boost_hash<size_t, 11>, // 5
		boost::bloom_filters::boost_hash<size_t, 13>, // 6
		boost::bloom_filters::boost_hash<size_t, 17>, // 7
		boost::bloom_filters::boost_hash<size_t, 19>, // 8
		boost::bloom_filters::boost_hash<size_t, 23>, // 9
		boost::bloom_filters::boost_hash<size_t, 29>, // 10
		boost::bloom_filters::boost_hash<size_t, 31>, // 11
		boost::bloom_filters::boost_hash<size_t, 37>, // 12
		boost::bloom_filters::boost_hash<size_t, 41>, // 13
		boost::bloom_filters::boost_hash<size_t, 43>, // 14
		boost::bloom_filters::boost_hash<size_t, 47>, // 15
		boost::bloom_filters::boost_hash<size_t, 53>, // 16
		boost::bloom_filters::boost_hash<size_t, 59>, // 17
		boost::bloom_filters::boost_hash<size_t, 61>, // 18
		boost::bloom_filters::boost_hash<size_t, 67>, // 19
		boost::bloom_filters::boost_hash<size_t, 71>, // 20
		boost::bloom_filters::boost_hash<size_t, 73>, // 21
		boost::bloom_filters::boost_hash<size_t, 79>, // 22
		boost::bloom_filters::boost_hash<size_t, 83>, // 23
		boost::bloom_filters::boost_hash<size_t, 89>, // 24
		boost::bloom_filters::boost_hash<size_t, 97>, // 25
		boost::bloom_filters::boost_hash<size_t, 101>, // 26
		boost::bloom_filters::boost_hash<size_t, 103>, // 27
		boost::bloom_filters::boost_hash<size_t, 107>, // 28
		boost::bloom_filters::boost_hash<size_t, 109>, // 29
		boost::bloom_filters::boost_hash<size_t, 113>, // 30
		boost::bloom_filters::boost_hash<size_t, 127>, // 31
		boost::bloom_filters::boost_hash<size_t, 131>, // 32
		boost::bloom_filters::boost_hash<size_t, 137>  // 33
> hash_function;
#endif

class SwordRT {

public:
	SwordRT();
	~SwordRT();

	inline bool Contains(size_t access, const char *filter_type);
	inline void Insert(size_t access, uint64_t tid, const char *filter_type);
	inline void CheckMemoryAccess(size_t access, size_t pc, AccessSize access_size, AccessType access_type, const char *nutex_name = "");
	inline void ReportRace(size_t access, size_t pc, uint64_t tid, AccessSize access_size, AccessType access_type, const char *nutex_name = "");
	void clear();

private:
	std::unordered_map<std::string, boost::bloom_filters::counting_bloom_filter<size_t, NUM_OF_ITEMS, TID_NUM_OF_BITS, hash_function>> filters;
	boost::bloom_filters::basic_bloom_filter<size_t, NUM_OF_REPORTED_RACES, hash_function> reported_races;

};

#endif  // SWORDRT_RTL_H
