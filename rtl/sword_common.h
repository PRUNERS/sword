#ifndef SWORD_COMMON_H
#define SWORD_COMMON_H

#include "minilzo.h"
#include <omp.h>
#include <ompt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

std::mutex pmtx;
std::mutex smtx;

#define SWORD_DEBUG 	1
#ifdef SWORD_DEBUG
#define ASSERT(x) assert(x);
#define DEBUG(stream, x) 												\
		do {															\
			std::unique_lock<std::mutex> lock(pmtx);					\
			stream << "DEBUG INFO[" << x << "][" << 					\
			__FUNCTION__ << ":" << __FILE__ << ":" << 					\
			std::dec << __LINE__ << "]" << std::endl;					\
		} while(0)
#define INFO(stream, x) 												\
		do {															\
			std::unique_lock<std::mutex> lock(pmtx);					\
			stream << x << std::endl;									\
		} while(0)
#else
#define ASSERT(x)
#define DEBUG(stream, x)
#endif

#define HEAP_ALLOC(var,size) thread_local lzo_align_t __LZO_MMODEL 		\
	var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);

#define SWORD_DATA 				"sword_data"
#define OFFSET_SPAN_FORMAT		"%01d%01d"
#define NUM_OF_ACCESSES			30000
#define BLOCK_SIZE 				NUM_OF_ACCESSES * sizeof(TraceItem)
#define MB_LIMIT 				BLOCK_SIZE
#define OUT_LEN     			(BLOCK_SIZE + BLOCK_SIZE / 16 + 64 + 3 + sizeof(lzo_uint)) // 8 byte to store the size of the block

enum AccessSize {
	size1 = 0,
	size2,
	size4,
	size8,
	size16
};

enum AccessType {
	unsafe_read = 0,
	unsafe_write,
	atomic_read,
	atomic_write,
};

static const char * AccessTypeStrings[] = { "Read", "Write", "Atomic Read", "Atomic Write" };

struct __attribute__ ((__packed__)) Access {
private:
	uint8_t size_type; // size in first 4 bits, type in last 4 bits
	size_t address;
	size_t pc;

public:
	Access() {
		size_type = 0;
		address = 0;
		pc = 0;
	}

	Access(AccessSize as, AccessType at, size_t a, size_t p) {
		size_type = (as << 4);
		size_type |= at;
		address = a;
		pc = p;
	}

	void setData(AccessSize as, AccessType at,
			size_t a, size_t p) {
		address = a;
		size_type = (as << 4);
		size_type |= at;
		pc = p;
	}

	AccessSize getAccessSize() const {
		return (AccessSize) (size_type >> 4);
	}

	AccessType getAccessType() const {
		return (AccessType) (size_type & 0x0F);
	}

	size_t getAddress() const {
		return address;
	}

	size_t getPC() const {
		return pc;
	}
};

struct __attribute__ ((__packed__)) Parallel {
private:
	ompt_id_t parallel_id;
	unsigned int team_size;

public:
	Parallel() {
		parallel_id = 0;
		team_size = 0;
	}

	Parallel(ompt_id_t pid, unsigned int ts) {
		parallel_id = pid;
		team_size = ts;
	}
};

struct __attribute__ ((__packed__)) Work {
private:
	ompt_work_type_t wstype;
	ompt_scope_endpoint_t endpoint;

public:
	Work() {
		wstype = ompt_work_loop;
		endpoint = ompt_scope_begin;
	}

	Work(ompt_work_type_t wt, ompt_scope_endpoint_t ep) {
		wstype = wt;
		endpoint = ep;
	}
};

struct __attribute__ ((__packed__)) Master {
private:
	ompt_scope_endpoint_t endpoint;

public:
	Master() {
		endpoint = ompt_scope_begin;
	}

	Master(ompt_scope_endpoint_t ep) {
		endpoint = ep;
	}
};

struct __attribute__ ((__packed__)) SyncRegion {
private:
	ompt_sync_region_kind_t kind;
	ompt_scope_endpoint_t endpoint;
	ompt_id_t barrier_id;

public:
	SyncRegion() {
		kind = ompt_sync_region_barrier;
		endpoint = ompt_scope_begin;
		barrier_id = 0;
	}

	SyncRegion(ompt_id_t bid, ompt_sync_region_kind_t k, ompt_scope_endpoint_t ep) {
		barrier_id = bid;
		kind = k;
		endpoint = ep;
	}
};

struct __attribute__ ((__packed__)) MutexRegion {
private:
	ompt_mutex_kind_t kind;
	ompt_wait_id_t wait_id;

public:
	MutexRegion() {
		kind = ompt_mutex;
		wait_id = 0;
	}

	MutexRegion(ompt_mutex_kind_t k, ompt_wait_id_t wid) {
		kind = k;
		wait_id = wid;
	}

	ompt_wait_id_t getWaitId() {
		return wait_id;
	}
};

struct __attribute__ ((__packed__)) TaskCreate {
private:
	ompt_id_t task_id;
	ompt_task_type_t type;
	int has_dependences;

public:
	TaskCreate() {
		task_id = 0;
		type = ompt_task_initial;
		has_dependences = 0;
	}

	TaskCreate(ompt_id_t id, ompt_task_type_t t, int hd) {
		task_id = id;
		type = t;
		has_dependences = hd;
	}
};

struct __attribute__ ((__packed__)) TaskSchedule {
private:
	ompt_id_t task_id;
	ompt_task_status_t status;

public:
	TaskSchedule() {
		task_id = 0;
		status = ompt_task_complete;
	}

	TaskSchedule(ompt_id_t id, ompt_task_status_t s) {
		task_id = id;
		status = s;
	}

	ompt_id_t getTaskID() {
		return task_id;
	}

	ompt_task_status_t getStatus() {
		return status;
	}
};

struct __attribute__ ((__packed__)) TaskDependence {
private:
	ompt_id_t task_id;
	void *variable_addr;
	unsigned int dependence_flags;

public:
	TaskDependence() {
		task_id = 0;
		variable_addr = NULL;
		dependence_flags = 0;
	}

	TaskDependence(ompt_id_t id, void *va, unsigned int df) {
		task_id = id;
		variable_addr = NULL;
		dependence_flags = 0;
	}
};

struct __attribute__ ((__packed__)) OffsetSpan {
private:
	unsigned offset;
	unsigned span;

public:
	OffsetSpan() {
		offset = 0;
		span = 0;
	}

	OffsetSpan(unsigned o, unsigned l) {
		offset = o;
		span = l;
	}

	unsigned getOffset() {
		return offset;
	}

	unsigned getSpan() {
			return span;
	}
};

enum CallbackType {
	data_access = 0, // 0: Access
	parallel_begin, // 1: Parallel: parallel id
	parallel_end, // 2: Parallel: parallel id
	work, // 3: Work: work type and endpoint
	master, // 4 :Master: only endpoint needed
	sync_region, // 5: SyncRegion: kind, endpoint and barrier id (for offset-span label)
	mutex_acquired, // 6: MutexRegion: kind and wait id
	mutex_released, // 7: MutexRegion: kind and wait id
	task_create, // 8: Task: type and has dependences
	task_schedule, // 9: TaskCreate: type and has dependences
	task_dependence, // 10: TaskDependences: type and has dependences
	os_label // 11: OffsetSpan: offset and span
};

struct TraceItem {
private:
	uint8_t item_type;

public:
	TraceItem() = default;

	void setType(CallbackType t) {
		item_type = (uint8_t) t;
	}

	CallbackType getType() const {
		return (CallbackType) item_type;
	}

	union {
		struct Access access;
		struct Parallel parallel;
		struct Work work;
		struct Master master;
		struct SyncRegion sync_region;
		struct MutexRegion mutex_region;
		struct TaskCreate task_create;
		struct TaskSchedule task_schedule;
		struct TaskDependence task_dependence;
		struct OffsetSpan offset_span;
	} data;
};

#endif  // SWORD_COMMON_H
