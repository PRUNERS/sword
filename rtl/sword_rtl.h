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

struct TaskData {
private:
	ompt_id_t task_id;

public:
	TaskData() {
		task_id = 0;
	}

	TaskData(ompt_id_t id) {
		task_id = id;
	}

	ompt_id_t getTaskID() {
		return task_id;
	}
};

struct ParallelData {
private:
	unsigned freed;
	unsigned state;
	ompt_id_t parallel_id;
	ompt_id_t parent_parallel_id;
	unsigned level;
	unsigned offset;
	unsigned span;

public:
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
		freed = 0;
		state = pd->getState();
		parallel_id = pd->getParallelID();
		parent_parallel_id = pd->getParentParallelID();
		level = pd->getParallelLevel();
		offset = pd->getOffset();
		span = pd->getSpan();
	}

	void setData(ParallelData *pd) {
		freed = pd->getFreed();
		state = pd->getState();
		parallel_id = pd->getParallelID();
		parent_parallel_id = pd->getParentParallelID();
		level = pd->getParallelLevel();
		offset = pd->getOffset();
		span = pd->getSpan();
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

	void setFreed(unsigned f) {
		freed = f;
	}

	void setState(unsigned v) {
		state = v;
	}

	void setParallelID(ompt_id_t pid) {
		parallel_id = pid;
	}

	void setParentParallelID(ompt_id_t ppid) {
		parent_parallel_id = ppid;
	}

	void setParallelLevel(unsigned l) {
		level = l;
	}

	void setOffset(unsigned o) {
		offset = o;
	}

	void setSpan(unsigned s) {
		span = s;
	}

	unsigned getFreed() {
		return freed;
	}

	unsigned getState() {
		return state;
	}

	ompt_id_t getParallelID() {
		return parallel_id;
	}

	ompt_id_t getParentParallelID() {
		return parent_parallel_id;
	}

	unsigned getParallelLevel() {
		return level;
	}

	unsigned getOffset() {
		return offset;
	}

	unsigned getSpan() {
		return span;
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
extern thread_local ParallelData *pdata;

typedef emilib::HashSet<uint64_t, NUM_OF_ACCESSES> fast_set;
thread_local fast_set set;

thread_local unsigned char *out;
thread_local std::future<bool> fut;

#endif  // SWORD_RTL_H
