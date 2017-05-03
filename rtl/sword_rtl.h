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

// #include <boost/unordered_set.hpp>
//#include "lazy/memory/buffer_allocator.h"
//#include "alb/stack_allocator.hpp"

#include <fcntl.h>
#include <sys/stat.h>

#include <future>
#include <unordered_set>
#include <vector>

#include <sparsehash/dense_hash_set>

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
	unsigned state;
	ompt_id_t parallel_id;
	unsigned level;
	std::string path;
	unsigned offset;
	unsigned span;

public:
	ParallelData() {
		state = 0;
		parallel_id = 0;
		level = 0;
		path = "";
		offset = 0;
		span = 0;
	}

	ParallelData(ompt_id_t pid, unsigned l, std::string p, unsigned o, unsigned s) {
		state = 0;
		parallel_id = pid;
		level = l;
		path = p;
		offset = o;
		span = s;
	}

	ParallelData(ParallelData *pd) {
		state = pd->getState();
		parallel_id = pd->getParallelID();
		level = pd->getParallelLevel();
		path = pd->getPath();
		offset = pd->getOffset();
		span = pd->getSpan();
	}

	void setData(ParallelData *pd) {
		state = pd->getState();
		parallel_id = pd->getParallelID();
		level = pd->getParallelLevel();
		path = pd->getPath();
		offset = pd->getOffset();
		span = pd->getSpan();
	}

	void setData(ompt_id_t pid, unsigned l, std::string p, unsigned o, unsigned s) {
		state = 0;
		parallel_id = pid;
		level = l;
		path = p;
		offset = o;
		span = s;
	}

	void setState(unsigned v) {
		state = v;
	}

	void setPath(std::string p) {
		path = p;
	}

	void setParallelID(ompt_id_t pid) {
		parallel_id = pid;
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

	unsigned getState() {
		return state;
	}

	ompt_id_t getParallelID() {
		return parallel_id;
	}

	unsigned getParallelLevel() {
		return level;
	}

	std::string getPath() {
		return path;
	}

	std::string getPath(int level) {
		switch(level) {
		case 0:
			return path;
		case -1:
			return path.substr(0, path.find_last_of("/"));
		default:
			return path;
		}
	}

	unsigned getOffset() {
		return offset;
	}

	unsigned getSpan() {
		return span;
	}
};

// Global Variables
// std::atomic<ompt_id_t> current_parallel_idx(0);

//typedef uint64_t key_type;
//typedef lazy::memory::buffer_allocator<key_type> allocator_type;
//typedef std::unordered_set<key_type, std::hash<key_type>, std::equal_to<key_type>, allocator_type> set_type;
//
//thread_local char buff[(NUM_OF_ACCESSES) * sizeof(uint64_t) * 20];
//thread_local allocator_type allocator(NUM_OF_ACCESSES * sizeof(uint64_t));
//std::hash<key_type> hasher;
//std::equal_to<key_type> cmp;
//thread_local set_type set(NUM_OF_ACCESSES, hasher, cmp, allocator);

// Thread Local Variables

extern thread_local int tid;
extern thread_local int __sword_status__;
extern thread_local std::vector<TraceItem> *accesses;
extern thread_local std::vector<TraceItem> *accesses1;
extern thread_local std::vector<TraceItem> *accesses2;
//thread_local std::unordered_set<uint64_t> set(NUM_OF_ACCESSES);
//extern thread_local TraceItem *accesses;
//extern thread_local TraceItem *accesses1;
//extern thread_local TraceItem *accesses2;
extern thread_local uint64_t idx;
extern thread_local uint64_t bid;
extern thread_local char *buffer;
// extern thread_local size_t *stack;
// extern thread_local size_t stacksize;
extern thread_local size_t offset;
extern thread_local FILE *datafile;
extern thread_local ParallelData *pdata;

thread_local google::dense_hash_set<uint64_t> set(NUM_OF_ACCESSES);
thread_local unsigned char *out;
thread_local std::future<bool> fut;
// thread_local int __sword_ignore_access = 0;

#endif  // SWORD_RTL_H
