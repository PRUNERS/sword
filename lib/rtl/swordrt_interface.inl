// UTIL

void __swordrt_init(uint64_t *access_min, uint64_t *access_max,
		uint64_t *function_min, uint64_t *function_max) {
	// INFO(std::cout, min << ":" << max);
	access_tsan_enabled = false;
	entry_tsan_enabled = false;

	std::string entry_filename = std::string(ARCHER_DATA) + "/entry_tsan_checks";
	if((fopen(entry_filename.c_str(), "r")) != NULL) {
		std::string access_filename = std::string(ARCHER_DATA) + "/access_tsan_checks";
		if((fopen(access_filename.c_str(), "r")) != NULL) {
			handle = dlopen("/home/simone/usr/lib/libsword-rt_tsan_strong.so", RTLD_LAZY);
			if (!handle) {
				fputs (dlerror(), stderr);
				exit(-1);
			}

			// DEBUG(std::cout, "Calling tsan_init");
			__tsan_init();
			access_tsan_enabled = true;
			entry_tsan_enabled = true;
		}
	}

	if(*access_min < swordrt_accesses_min)
		swordrt_accesses_min = *access_min;
	if(*access_max > swordrt_accesses_max)
		swordrt_accesses_max = *access_max;

	if(*function_min < swordrt_function_min)
		swordrt_function_min = *function_min;
	if(*function_max > swordrt_function_max)
		swordrt_function_max = *function_max;
}

int __swordomp_get_status() {
	return __swordomp_status__;
}

void __swordomp_status_inc() {
	__swordomp_status__++;
}

void __swordomp_status_dec() {
	__swordomp_status__++;
}

void __swordomp_func_entry(uint64_t hash, void *pc) {
	__swordrt_hash__ = hash;
	// if tsan is enabled and hash in list of tsan checks than call __tsan_func_entry
	if(entry_tsan_enabled) {
		std::unordered_set<uint64_t>::iterator it = entry_tsan_checks.find(hash);
		if(it != entry_tsan_checks.end())
			__tsan_func_entry(pc);
	}
}

void __swordomp_func_exit(uint64_t hash) {
	// if tsan is enabled and hash in list of tsan checks than call __tsan_func_exit
	if(entry_tsan_enabled) {
		std::unordered_set<uint64_t>::iterator it = entry_tsan_checks.find(hash);
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

	if(!__swordomp_is_critical__) {
		SAVE_ACCESS(size1, unsafe_read)
	} else {
		SAVE_ACCESS(size1, mutex_read)
	}
}


void __swordomp_read2(void *addr, uint64_t hash) {
	TSAN_CHECK(read2);

	DEF_ACCESS
	CHECK_STACK

	if(!__swordomp_is_critical__) {
		SAVE_ACCESS(size2, unsafe_read)
	} else {
		SAVE_ACCESS(size2, mutex_read)
	}
}

void __swordomp_read4(void *addr, uint64_t hash) {
	TSAN_CHECK(read4);

	DEF_ACCESS
	CHECK_STACK

	if(!__swordomp_is_critical__) {
		SAVE_ACCESS(size4, unsafe_read)
	} else {
		SAVE_ACCESS(size4, mutex_read)
	}
}

void __swordomp_read8(void *addr, uint64_t hash) {
	TSAN_CHECK(read8);

	DEF_ACCESS
	CHECK_STACK

	if(!__swordomp_is_critical__) {
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

	if(!__swordomp_is_critical__) {
		SAVE_ACCESS(size1, unsafe_write)
	} else {
		SAVE_ACCESS(size1, mutex_write)
	}
}

void __swordomp_write2(void *addr, uint64_t hash) {
	TSAN_CHECK(write2);

	DEF_ACCESS
	CHECK_STACK

	if(!__swordomp_is_critical__) {
		SAVE_ACCESS(size2, unsafe_write)
	} else {
		SAVE_ACCESS(size2, mutex_write)
	}
}

void __swordomp_write4(void *addr, uint64_t hash) {
	TSAN_CHECK(write4);

	DEF_ACCESS
	CHECK_STACK

	if(!__swordomp_is_critical__) {
		SAVE_ACCESS(size4, unsafe_write)
	} else {
		SAVE_ACCESS(size4, mutex_write)
	}
}

void __swordomp_write8(void *addr, uint64_t hash) {
	TSAN_CHECK(write8);

	DEF_ACCESS
	CHECK_STACK

	if(!__swordomp_is_critical__) {
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
