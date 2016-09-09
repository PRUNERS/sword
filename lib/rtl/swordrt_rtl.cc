//===-- swordrt_rtl.cc -------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of Archer/SwordRT, an OpenMP race detector.
//
// Main file (entry points) for the TSan run-time.
//===----------------------------------------------------------------------===//

#include "swordrt_rtl.h"

SwordRT *swordRT;

// Class SwordRT

SwordRT::SwordRT() {
	InitializeBloomFilterParameters();

	filters.insert({ UNSAFE_READ , new bloom_filter(parameters) });
	filters.insert({ UNSAFE_WRITE, new bloom_filter(parameters) });
	filters.insert({ ATOMIC_READ , new bloom_filter(parameters) });
	filters.insert({ ATOMIC_WRITE, new bloom_filter(parameters) });
	filters.insert({ MUTEX_READ  , new bloom_filter(parameters) });
	filters.insert({ MUTEX_WRITE , new bloom_filter(parameters) });

	c = new CountMinSketch(0.01, 0.001);
}

SwordRT::~SwordRT() {
	// filters.clear();
}

void SwordRT::InitializeBloomFilterParameters() {
	// Initialize Bloom Filters
	// This number should be roughly the number of different addresses
	// that a parallel region access during its execution, how do we
	// find it out?
	parameters.projected_element_count = 100000;

	// What maximum tolerable false positive probability? (0,1)
	parameters.false_positive_probability = 0.001; // 1 in 1000

	// Simple randomizer (optional) - Try different seeds?
	parameters.random_seed = 0xA5A5A5A5;

	if (!parameters) {
		std::cerr << "Error - Invalid set of bloom filter parameters!" << std::endl;
		exit(1);
	}

	parameters.compute_optimal_parameters();
}

bool ALWAYS_INLINE SwordRT::Contains(uint64_t access, std::string filter_type) {
	return filters[filter_type]->contains(access);
}

void ALWAYS_INLINE SwordRT::Insert(uint64_t access, std::string filter_type) {
	filters[filter_type]->insert(access);
}

bool ALWAYS_INLINE SwordRT::ContainsAndInsert(uint64_t access, std::string filter_type) {
	bool res = filters[filter_type]->contains(access);
	filters[filter_type]->insert(access);
	return res;
}

inline void SwordRT::ReportRace(uint64_t access, uint64_t pc, AccessSize access_size, AccessType access_type, const char *nutex_name) {
	// Will call a class that executes llvm-symbolizer at the end of each parallel region,
	// here we just keep filling up the file that holds all the executable/addresses
	// We also put the address of a parallel region so we know the parallel region where the
	// access belongs to
	std::cerr << "There was a race at [" << std::hex << (void *) access << "][" << (void *) pc << "]" << std::endl;
}

bloom_parameters SwordRT::getParameters() {
	return parameters;
}

void SwordRT::clear() {
	for (auto& f: filters) {
		f.second->clear();
	}
}

void ALWAYS_INLINE SwordRT::CheckMemoryAccess(uint64_t access, uint64_t pc, AccessSize access_size, AccessType access_type, const char *nutex_name) {
	bool conflict = false;
	AccessType conflict_type = none;
	std::string nutex;

	// printf("AccessType: %s[0x%lx]\n", FilterType[access_type], access);
	// printf("[0x%lx]%d\n", access, omp_get_thread_num());
	// std::cout << "[" << std::hex << access << "]" << omp_get_thread_num() << std::endl;

	switch(access_type) {
	case unsafe_read:
		if(swordRT->Contains(access, UNSAFE_WRITE)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, ATOMIC_WRITE)) {
			conflict = true;
			conflict_type = atomic_write;
		} else if(swordRT->Contains(access, MUTEX_WRITE)) {
			conflict = true;
			conflict_type = mutex_write;
		}
		break;
	case unsafe_write:
		if(swordRT->Contains(access, UNSAFE_READ)) {
			conflict = true;
			conflict_type = unsafe_read;
		} else if(swordRT->Contains(access, UNSAFE_WRITE)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, ATOMIC_READ)) {
			conflict = true;
			conflict_type = atomic_read;
		} else if(swordRT->Contains(access, ATOMIC_WRITE)) {
			conflict = true;
			conflict_type = atomic_write;
		} else if(swordRT->Contains(access, MUTEX_READ)) {
			conflict = true;
			conflict_type = mutex_read;
		} else if(swordRT->Contains(access, MUTEX_WRITE)) {
			conflict = true;
			conflict_type = mutex_write;
		}
		break;
	case atomic_read:
		if(swordRT->Contains(access, UNSAFE_WRITE)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, MUTEX_WRITE)) {
			conflict = true;
			conflict_type = mutex_write;
		}
		break;
	case atomic_write:
		if(swordRT->Contains(access, UNSAFE_READ)) {
			conflict = true;
			conflict_type = unsafe_read;
		} else if(swordRT->Contains(access, UNSAFE_WRITE)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, MUTEX_READ)) {
			conflict = true;
			conflict_type = mutex_read;
		} else if(swordRT->Contains(access, MUTEX_WRITE)) {
			conflict = true;
			conflict_type = mutex_write;
		}
		break;
	case mutex_read:
		if(swordRT->Contains(access, UNSAFE_WRITE)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, ATOMIC_WRITE)) {
			conflict = true;
			conflict_type = atomic_write;
		}
		break;
	case mutex_write:
		if(swordRT->Contains(access, UNSAFE_READ)) {
			conflict = true;
			conflict_type = unsafe_read;
		} else if(swordRT->Contains(access, UNSAFE_WRITE)) {
			conflict = true;
			conflict_type = unsafe_write;
		} else if(swordRT->Contains(access, ATOMIC_READ)) {
			conflict = true;
			conflict_type = atomic_read;
		} else if(swordRT->Contains(access, ATOMIC_WRITE)) {
			conflict = true;
			conflict_type = atomic_write;
		}
		break;
	case nutex_read:
		break;
	case nutex_write:
		break;
	default:
		return;
	}

	// Trying to manage the thread races with itself
	//	if(!tls_filter)
	//		tls_filter = new bloom_filter(swordRT->parameters);
	//	if(tls_filter->contains(access)) {
	//		conflict = false;
	//	}
	//
	//	tls_filter->insert(access);

	int tid = omp_get_thread_num();
	char str[8];
	sprintf(str, "%lx", access);
	if(c->estimate(str) == tid)
		conflict = false;

	swordRT->Insert(access, FilterType[access_type]);
	c->update(str, tid);
	if(conflict)
		ReportRace(access, pc, access_size, access_type, nutex_name);
}

// Class SwordRT

extern "C" {

// READS
void __swordomp_read1(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size1, mutex_read);
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size1, unsafe_read);
}

void __swordomp_read2(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size2, mutex_read);
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size2, unsafe_read);
}

void __swordomp_read4(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_read);
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_read);
}

void __swordomp_read8(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size8, mutex_read);
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size8, unsafe_read);
}
// READS

// WRITES
void __swordomp_write1(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size1, mutex_write);
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size1, unsafe_write);
}

void __swordomp_write2(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size2, mutex_write);
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size2, unsafe_write);
}

void __swordomp_write4(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_write);
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_write);
}

void __swordomp_write41(void *addr) {
	std::cout << "WRITE4[" << std::hex << addr << "]" << std::endl;
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_write);
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_write);
}

void __swordomp_write8(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size8, mutex_write);
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size8, unsafe_write);
}
// WRITES

// ATOMICS

// LOAD
void __swordomp_atomic8_load(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size1, atomic_read);
}

void __swordomp_atomic16_load(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size2, atomic_read);
}

void __swordomp_atomic32_load(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, atomic_read);
}

void __swordomp_atomic64_load(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size8, atomic_read);
}

void __swordomp_atomic128_load(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size16, atomic_read);
}
// LOAD

// STORE
void __swordomp_atomic8_store(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size1, atomic_write);
}

void __swordomp_atomic16_store(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size2, atomic_write);
}

void __swordomp_atomic32_store(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, atomic_write);
}

void __swordomp_atomic64_store(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size8, atomic_write);
}

void __swordomp_atomic128_store(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size16, atomic_write);
}
// STORE

// ADD
void __swordomp_atomic8_fetch_add(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size1, atomic_write);
}

void __swordomp_atomic16_fetch_add(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size2, atomic_write);
}

void __swordomp_atomic32_fetch_add(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, atomic_write);
}

void __swordomp_atomic64_fetch_add(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size8, atomic_write);
}

void __swordomp_atomic128_fetch_add(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size16, atomic_write);
}
// ADD

// SUB
void __swordomp_atomic8_fetch_sub(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size1, atomic_write);
}

void __swordomp_atomic16_fetch_sub(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size2, atomic_write);
}

void __swordomp_atomic32_fetch_sub(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, atomic_write);
}

void __swordomp_atomic64_fetch_sub(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size8, atomic_write);
}

void __swordomp_atomic128_fetch_sub(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size16, atomic_write);
}
// SUB

// COMPARE EXCHANGE
void __swordomp_atomic8_compare_exchange_val(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size1, atomic_write);
}

void __swordomp_atomic16_compare_exchange_val(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size2, atomic_write);
}

void __swordomp_atomic32_compare_exchange_val(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, atomic_write);
}

void __swordomp_atomic64_compare_exchange_val(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size8, atomic_write);
}

void __swordomp_atomic128_compare_exchange_val(void *addr) {
	swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size16, atomic_write);
}
// COMPARE EXCHANGE

// ATOMICS

static void on_ompt_event_parallel_begin(ompt_task_id_t parent_task_id,
		ompt_frame_t *parent_task_frame,
		ompt_parallel_id_t parallel_id,
		uint32_t requested_team_size,
		void *parallel_function,
		ompt_invoker_t invoker) {
	// printf("BEGIN ThreadID: %d\n", omp_get_thread_num());
}

static void on_ompt_event_parallel_end(ompt_parallel_id_t parallel_id,
		ompt_task_id_t task_id,
		ompt_invoker_t invoker) {

	// printf("END ThreadID: %d\n", omp_get_thread_num());
	if(__swordomp_status__ == 0) {
		swordRT->clear();
	}
}

static void on_acquired_critical(ompt_wait_id_t wait_id) {
	__swordomp_is_critical__ = true;
}

static void on_release_critical(ompt_wait_id_t wait_id) {
	__swordomp_is_critical__ = false;
}

static void ompt_initialize_fn(ompt_function_lookup_t lookup,
		const char *runtime_version,
		unsigned int ompt_version) {
	swordRT = new SwordRT();

	if(!swordRT) {
		std::cerr << "Error initializing the runtime!" << std::endl;
		exit(1);
	}

	printf("OMPT Initialization...\n");
	printf("runtime_version: %s, ompt_version: %i\n", runtime_version, ompt_version);

	ompt_set_callback_t ompt_set_callback = (ompt_set_callback_t) lookup("ompt_set_callback");
	// ompt_get_thread_id = (ompt_get_thread_id_t) lookup("ompt_get_thread_id");

	ompt_set_callback(ompt_event_parallel_begin,
			(ompt_callback_t) &on_ompt_event_parallel_begin);
	ompt_set_callback(ompt_event_parallel_end,
			(ompt_callback_t) &on_ompt_event_parallel_end);
	// ompt_set_callback(ompt_event_barrier_begin,
	//                   (ompt_callback_t) &on_ompt_event_barrier_begin);
	// ompt_set_callback(ompt_event_barrier_end,
	//                   (ompt_callback_t) &on_ompt_event_barrier_end);
	ompt_set_callback(ompt_event_acquired_critical,
			(ompt_callback_t) &on_acquired_critical);
	ompt_set_callback(ompt_event_release_critical,
			(ompt_callback_t) &on_release_critical);
}

ompt_initialize_t ompt_tool(void) { return ompt_initialize_fn; }

}
