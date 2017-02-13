#ifndef SWORD_COMMON_H
#define SWORD_COMMON_H

#include <omp.h>
#include <ompt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


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

public:
	Parallel() {
		parallel_id = 0;
	}

	Parallel(ompt_id_t pid) {
		parallel_id = pid;
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
};

enum CallbackType {
	data_access = 0, // Access
	parallel_begin, // Parallel: parallel id
	parallel_end, // Parallel: parallel id
	work, // Work: work type and endpoint
	master, // Master: only endpoint needed
	sync_region, // SyncRegion: kind, endpoint and barrier id (for offset-span label)
	mutex_acquired, // MutexRegion: kind and wait id
	mutex_released // MutexRegion: kind and wait id
};

struct TraceItem {
private:
	uint8_t type;

public:
	void setType(CallbackType t) {
		type = (uint8_t) t;
	}

	CallbackType getType() const {
		return (CallbackType) type;
	}

	union {
		struct Access access;
		struct Parallel parallel;
		struct Work work;
		struct Master master;
		struct SyncRegion sync_region;
		struct MutexRegion mutex_region;
	} data;
};

#endif  // SWORD_COMMON_H
