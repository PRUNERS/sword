// UTIL

void __swordrt_init(uint64_t *access_min, uint64_t *access_max,
		uint64_t *function_min, uint64_t *function_max) {
	__ompsan_init();
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

void __swordomp_func_entry(uint64_t hash, void *pc) {}

void __swordomp_func_exit(uint64_t hash) {}

// UTIL

// READS
void __swordomp_read1(void *addr, uint64_t hash) {
	DEF_ACCESS

	if(!__swordomp_is_critical__[tid]) {
		SAVE_ACCESS(read1, size1, unsafe_read)
	} else {
		SAVE_ACCESS(read1, size1, mutex_read)
	}
}


void __swordomp_read2(void *addr, uint64_t hash) {
	DEF_ACCESS

	if(!__swordomp_is_critical__[tid]) {
		SAVE_ACCESS(read2, size2, unsafe_read)
	} else {
		SAVE_ACCESS(read2, size2, mutex_read)
	}
}

void __swordomp_read4(void *addr, uint64_t hash) {
	DEF_ACCESS

	if(!__swordomp_is_critical__[tid]) {
		SAVE_ACCESS(read4, size4, unsafe_read)
	} else {
		SAVE_ACCESS(read4, size4, mutex_read)
	}
}

void __swordomp_read8(void *addr, uint64_t hash) {
	DEF_ACCESS

	if(!__swordomp_is_critical__[tid]) {
		SAVE_ACCESS(read8, size8, unsafe_read)
	} else {
		SAVE_ACCESS(read8, size8, mutex_read)
	}
}

void __swordomp_read16(void *addr, uint64_t hash) {
	DEF_ACCESS

	if(!__swordomp_is_critical__[tid]) {
		SAVE_ACCESS(read16, size16, unsafe_read)
	} else {
		SAVE_ACCESS(read16, size16, mutex_read)
	}
}
// READS

// WRITES
void __swordomp_write1(void *addr, uint64_t hash) {
	DEF_ACCESS

	if(!__swordomp_is_critical__[tid]) {
		SAVE_ACCESS(write1, size1, unsafe_write)
	} else {
		SAVE_ACCESS(write1, size1, mutex_write)
	}
}

void __swordomp_write2(void *addr, uint64_t hash) {
	DEF_ACCESS

	if(!__swordomp_is_critical__[tid]) {
		SAVE_ACCESS(write2, size2, unsafe_write)
	} else {
		SAVE_ACCESS(write2, size2, mutex_write)
	}
}

void __swordomp_write4(void *addr, uint64_t hash) {
	DEF_ACCESS

	if(!__swordomp_is_critical__[tid]) {
		SAVE_ACCESS(write4, size4, unsafe_write)
	} else {
		SAVE_ACCESS(write4, size4, mutex_write)
	}
}

void __swordomp_write8(void *addr, uint64_t hash) {
	DEF_ACCESS

	if(!__swordomp_is_critical__[tid]) {
		SAVE_ACCESS(write8, size8, unsafe_write)
	} else {
		SAVE_ACCESS(write8, size8, mutex_write)
	}
}

void __swordomp_write16(void *addr, uint64_t hash) {
	DEF_ACCESS

	if(!__swordomp_is_critical__[tid]) {
		SAVE_ACCESS(write16, size16, unsafe_write)
	} else {
		SAVE_ACCESS(write16, size16, mutex_write)
	}
}
// WRITES

// ATOMICS

// LOAD

void __swordomp_atomic8_load(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic8_load, size1, atomic_read)
}

void __swordomp_atomic16_load(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic16_load)
}

void __swordomp_atomic32_load(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic32_load, size4, atomic_read)
}

void __swordomp_atomic64_load(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic64_load, size8, atomic_read)
}

void __swordomp_atomic128_load(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic128_load, size16, atomic_read)
}
// LOAD

// STORE
void __swordomp_atomic8_store(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic8_store, size1, atomic_write)
}

void __swordomp_atomic16_store(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic16_store, size2, atomic_write)
}

void __swordomp_atomic32_store(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic32_store, size4, atomic_write)
}

void __swordomp_atomic64_store(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic64_store, size8, atomic_write)
}

void __swordomp_atomic128_store(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic128_store, size16, atomic_write)
}
// STORE

// ADD
void __swordomp_atomic8_fetch_add(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic8_fetch_add, size1, atomic_write)
}

void __swordomp_atomic16_fetch_add(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic8_fetch_add, size2, atomic_write)
}

void __swordomp_atomic32_fetch_add(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic32_fetch_add, size4, atomic_write)
}

void __swordomp_atomic64_fetch_add(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic64_fetch_add, size8, atomic_write)
}

void __swordomp_atomic128_fetch_add(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic128_fetch_add, size16, atomic_write)
}
// ADD

// SUB
void __swordomp_atomic8_fetch_sub(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic8_fetch_sub, size1, atomic_write)
}

void __swordomp_atomic16_fetch_sub(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic16_fetch_sub, size2, atomic_write)
}

void __swordomp_atomic32_fetch_sub(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic32_fetch_sub, size4, atomic_write)
}

void __swordomp_atomic64_fetch_sub(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic64_fetch_sub, size8, atomic_write)
}

void __swordomp_atomic128_fetch_sub(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic128_fetch_sub, size16, atomic_write)
}
// SUB

// COMPARE EXCHANGE
void __swordomp_atomic8_compare_exchange_val(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic8_compare_exchange_val, size1, atomic_write)
}

void __swordomp_atomic16_compare_exchange_val(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic16_compare_exchange_val, size2, atomic_write)
}

void __swordomp_atomic32_compare_exchange_val(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic32_compare_exchange_val, size4, atomic_write)
}

void __swordomp_atomic64_compare_exchange_val(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic64_compare_exchange_val, size8, atomic_write)
}

void __swordomp_atomic128_compare_exchange_val(void *addr, uint64_t hash) {
	DEF_ACCESS

	// SAVE_ACCESS(atomic128_compare_exchange_val, size16, atomic_write)
}

// COMPARE EXCHANGE

// ATOMICS
