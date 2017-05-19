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
#include "sword_hashset.h"

#include <fcntl.h>
#include <sys/stat.h>

#include <future>
#include <vector>

#define ALWAYS_INLINE			__attribute__((always_inline))
#define CALLERPC 				((size_t) __builtin_return_address(0))

struct ParallelData {
	unsigned freed;
	unsigned state;
	ompt_id_t parallel_id;
	ompt_id_t parent_parallel_id;
	unsigned level;
	unsigned offset;
	unsigned span;

	ParallelData() {
		freed = 0;
		state = 0;
		parallel_id = 0;
		parent_parallel_id = 0;
		level = 0;
		offset = 0;
		span = 0;
	}

	ParallelData(ompt_id_t pid, ompt_id_t ppid, unsigned l, unsigned o, unsigned s) {
		freed = 0;
		state = 0;
		parallel_id = pid;
		parent_parallel_id = ppid;
		level = l;
		offset = o;
		span = s;
	}

	ParallelData(ParallelData *pd) {
		freed = pd->freed;
		state = pd->state;
		parallel_id = pd->parallel_id;
		parent_parallel_id = pd->parent_parallel_id;
		level = pd->level;
		offset = pd->offset;
		span = pd->span;
	}

	void setData(ParallelData *pd) {
		freed = pd->freed;
		state = pd->state;
		parallel_id = pd->parallel_id;
		parent_parallel_id = pd->parent_parallel_id;
		level = pd->level;
		offset = pd->offset;
		span = pd->span;
	}

	void setData(ompt_id_t pid, ompt_id_t ppid, unsigned l, unsigned o, unsigned s) {
		freed = 0;
		state = 0;
		parallel_id = pid;
		parent_parallel_id = ppid;
		level = l;
		offset = o;
		span = s;
	}
};

extern thread_local int tid;
extern thread_local int __sword_status__;
extern thread_local std::vector<TraceItem> *accesses;
extern thread_local std::vector<TraceItem> *accesses1;
extern thread_local std::vector<TraceItem> *accesses2;
extern thread_local uint64_t idx;
extern thread_local uint64_t bid;
extern thread_local char *buffer;
extern thread_local size_t offset;
extern thread_local size_t file_offset_begin;
extern thread_local size_t file_offset_end;
extern thread_local FILE *datafile;
extern thread_local FILE *metafile;

typedef emilib::HashSet<uint64_t, NUM_OF_ACCESSES> fast_set;
thread_local fast_set set;

thread_local unsigned char *out;
thread_local std::future<bool> fut;

#endif  // SWORD_RTL_H
