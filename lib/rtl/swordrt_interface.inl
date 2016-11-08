#include <swordrt_tsan_interface.h>

// UTIL

void __swordrt_init() {
	tsan_enabled = false;
	FILE *fp;
	std::string filename = std::string(ARCHER_DATA) + "/tsanchecks";
	if((fp = fopen(filename.c_str(), "r")) != NULL) {
		DEBUG(std::cout, "Calling tsan_init");
		__tsan_init();
		tsan_enabled = true;
	}
}

int __swordomp_get_status() {
#if defined(TLS) || defined(NOTLS)
	return __swordomp_status__;
#else
	return threadInfo[tid - 1].__swordomp_status__;
#endif
}

void __swordomp_status_inc() {
#if defined(TLS) || defined(NOTLS)
	__swordomp_status__++;
#else
	threadInfo[tid - 1].__swordomp_status__++;
#endif
}

void __swordomp_status_dec() {
#if defined(TLS) || defined(NOTLS)
	__swordomp_status__++;
#else
	threadInfo[tid - 1].__swordomp_status__--;
#endif
}

void __swordomp_func_entry(uint64_t hash, void *pc) {
	__swordrt_hash__ = hash;
	// if tsan is enabled and hash in list of tsan checks than call __tsan_func_entry
	if(tsan_enabled) {
		std::set<uint64_t>::iterator it = entry_tsan_checks.find(hash);
		if(it != entry_tsan_checks.end())
			__tsan_func_entry(pc);
	}
}

void __swordomp_func_exit(uint64_t hash) {
	// if tsan is enabled and hash in list of tsan checks than call __tsan_func_exit
	if(tsan_enabled) {
		std::set<uint64_t>::iterator it = entry_tsan_checks.find(hash);
		if(it != entry_tsan_checks.end())
			__tsan_func_exit();
	}
}

// UTIL

// READS
void __swordomp_read1(void *addr, uint64_t hash) {
	TSAN_CHECK(read1);
	
	DEF_ACCESS
	CHECK_STACK

#if defined(TLS) || defined(NOTLS)
	if(!__swordomp_is_critical__) {
#else
	if(!threadInfo[tid - 1].__swordomp_is_critical__) {
#endif
		SAVE_ACCESS(size1, unsafe_read)
	} else {
		SAVE_ACCESS(size1, mutex_read)
	}
}
	

void __swordomp_read2(void *addr, uint64_t hash) {
	TSAN_CHECK(read2);

	DEF_ACCESS
	CHECK_STACK

#if defined(TLS) || defined(NOTLS)
	if(!__swordomp_is_critical__) {
#else
	if(!threadInfo[tid - 1].__swordomp_is_critical__) {
#endif
		SAVE_ACCESS(size2, unsafe_read)
	} else {
		SAVE_ACCESS(size2, mutex_read)
	}
}

void __swordomp_read4(void *addr, uint64_t hash) {
	TSAN_CHECK(read4);
			
	DEF_ACCESS
	CHECK_STACK

#if defined(TLS) || defined(NOTLS)
	if(!__swordomp_is_critical__) {
#else
	if(!threadInfo[tid - 1].__swordomp_is_critical__) {
#endif
		SAVE_ACCESS(size4, unsafe_read)
	} else {
		SAVE_ACCESS(size4, mutex_read)
	}
}

void __swordomp_read8(void *addr, uint64_t hash) {
	TSAN_CHECK(read8);
			
	DEF_ACCESS
	CHECK_STACK

#if defined(TLS) || defined(NOTLS)
	if(!__swordomp_is_critical__) {
#else
	if(!threadInfo[tid - 1].__swordomp_is_critical__) {
#endif
		SAVE_ACCESS(size8, unsafe_read)
	} else {
		SAVE_ACCESS(size8, mutex_read)
	}
}
// READS

// WRITES
void __swordomp_write1(void *addr, uint64_t hash) {
	TSAN_CHECK(write1);
			
	DEF_ACCESS
	CHECK_STACK

#if defined(TLS) || defined(NOTLS)
	if(!__swordomp_is_critical__) {
#else
	if(!threadInfo[tid - 1].__swordomp_is_critical__) {
#endif
		SAVE_ACCESS(size1, unsafe_write)
	} else {
		SAVE_ACCESS(size1, mutex_write)
	}
}

void __swordomp_write2(void *addr, uint64_t hash) {
	TSAN_CHECK(write2);
			
	DEF_ACCESS
	CHECK_STACK

#if defined(TLS) || defined(NOTLS)
	if(!__swordomp_is_critical__) {
#else
	if(!threadInfo[tid - 1].__swordomp_is_critical__) {
#endif
		SAVE_ACCESS(size2, unsafe_write)
	} else {
		SAVE_ACCESS(size2, mutex_write)
	}
}

void __swordomp_write4(void *addr, uint64_t hash) {
	TSAN_CHECK(write4);
			
	DEF_ACCESS
	CHECK_STACK

#if defined(TLS) || defined(NOTLS)
	if(!__swordomp_is_critical__) {
#else
	if(!threadInfo[tid - 1].__swordomp_is_critical__) {
#endif
		SAVE_ACCESS(size4, unsafe_write)
	} else {
		SAVE_ACCESS(size4, mutex_write)
	}
}

void __swordomp_write8(void *addr, uint64_t hash) {
	TSAN_CHECK(write8);
	
	DEF_ACCESS
	CHECK_STACK

#if defined(TLS) || defined(NOTLS)
	if(!__swordomp_is_critical__) {
#else
	if(!threadInfo[tid - 1].__swordomp_is_critical__) {
#endif
		SAVE_ACCESS(size8, unsafe_write)
	} else {
		SAVE_ACCESS(size8, mutex_write)
	}
}
// WRITES

// ATOMICS

// LOAD
void __swordomp_atomic8_load(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size1, atomic_read)
}

void __swordomp_atomic16_load(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size2, atomic_read)
}

void __swordomp_atomic32_load(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size4, atomic_read)
}

void __swordomp_atomic64_load(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size8, atomic_read)
}

void __swordomp_atomic128_load(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size16, atomic_read)
}
// LOAD

// STORE
void __swordomp_atomic8_store(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size1, atomic_write)
}

void __swordomp_atomic16_store(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size2, atomic_write)
}

void __swordomp_atomic32_store(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size4, atomic_write)
}

void __swordomp_atomic64_store(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size8, atomic_write)
}

void __swordomp_atomic128_store(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size16, atomic_write)
}
// STORE

// ADD
void __swordomp_atomic8_fetch_add(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size1, atomic_write)
}

void __swordomp_atomic16_fetch_add(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size2, atomic_write)
}

void __swordomp_atomic32_fetch_add(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size4, atomic_write)
}

void __swordomp_atomic64_fetch_add(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size8, atomic_write)
}

void __swordomp_atomic128_fetch_add(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size16, atomic_write)
}
// ADD

// SUB
void __swordomp_atomic8_fetch_sub(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size1, atomic_write)
}

void __swordomp_atomic16_fetch_sub(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size2, atomic_write)
}

void __swordomp_atomic32_fetch_sub(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size4, atomic_write)
}

void __swordomp_atomic64_fetch_sub(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size8, atomic_write)
}

void __swordomp_atomic128_fetch_sub(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size16, atomic_write)
}
// SUB

// COMPARE EXCHANGE
void __swordomp_atomic8_compare_exchange_val(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size1, atomic_write)
}

void __swordomp_atomic16_compare_exchange_val(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size2, atomic_write)
}

void __swordomp_atomic32_compare_exchange_val(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size4, atomic_write)
}

void __swordomp_atomic64_compare_exchange_val(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size8, atomic_write)
}

void __swordomp_atomic128_compare_exchange_val(void *addr, uint64_t hash) {
	DEF_ACCESS
	CHECK_STACK	
	SAVE_ACCESS(size16, atomic_write)
}
// COMPARE EXCHANGE

// ATOMICS
