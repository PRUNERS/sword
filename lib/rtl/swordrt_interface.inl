
// UTIL

int __swordomp_get_status() {
	return __swordomp_status__;
}

void __swordomp_status_inc() {
	__swordomp_status__++;
}

void __swordomp_status_dec() {
	__swordomp_status__--;
}

// UTIL

// READS
void __swordomp_read1(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;

	if(!__swordomp_is_critical__) {
		SAVE_ACCESS(access, size1, unsafe_read)
	} else {
		SAVE_ACCESS(access, size1, mutex_read)
	}
}

void __swordomp_read2(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;

	if(!__swordomp_is_critical__) {
		SAVE_ACCESS(access, size2, unsafe_read)
	} else {
		SAVE_ACCESS(access, size2, mutex_read)
	}
}

void __swordomp_read4(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;

	if(!__swordomp_is_critical__) {
		SAVE_ACCESS(access, size4, unsafe_read)
	} else {
		SAVE_ACCESS(access, size4, mutex_read)
	}
}

void __swordomp_read8(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;

	if(!__swordomp_is_critical__) {
		SAVE_ACCESS(access, size8, unsafe_read)
	} else {
		SAVE_ACCESS(access, size8, mutex_read)
	}
}
// READS

// WRITES
void __swordomp_write1(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;

	if(!__swordomp_is_critical__) {
		SAVE_ACCESS(access, size1, unsafe_write)
	} else {
		SAVE_ACCESS(access, size1, mutex_write)
	}
}

void __swordomp_write2(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;

	if(!__swordomp_is_critical__) {
		SAVE_ACCESS(access, size2, unsafe_write)
	} else {
		SAVE_ACCESS(access, size2, mutex_write)
	}
}

void __swordomp_write4(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;

	if(!__swordomp_is_critical__) {
		SAVE_ACCESS(access, size4, unsafe_write)
	} else {
		SAVE_ACCESS(access, size4, mutex_write)
	}
}

void __swordomp_write8(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;

	if(!__swordomp_is_critical__) {
		SAVE_ACCESS(access, size8, unsafe_write)
	} else {
		SAVE_ACCESS(access, size8, mutex_write)
	}
}
// WRITES

// ATOMICS

// LOAD
void __swordomp_atomic8_load(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size1, atomic_read)
}

void __swordomp_atomic16_load(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size2, atomic_read)
}

void __swordomp_atomic32_load(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size4, atomic_read)
}

void __swordomp_atomic64_load(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size8, atomic_read)
}

void __swordomp_atomic128_load(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size16, atomic_read)
}
// LOAD

// STORE
void __swordomp_atomic8_store(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size1, atomic_write)
}

void __swordomp_atomic16_store(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size2, atomic_write)
}

void __swordomp_atomic32_store(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size4, atomic_write)
}

void __swordomp_atomic64_store(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size8, atomic_write)
}

void __swordomp_atomic128_store(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size16, atomic_write)
}
// STORE

// ADD
void __swordomp_atomic8_fetch_add(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size1, atomic_write)
}

void __swordomp_atomic16_fetch_add(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size2, atomic_write)
}

void __swordomp_atomic32_fetch_add(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size4, atomic_write)
}

void __swordomp_atomic64_fetch_add(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size8, atomic_write)
}

void __swordomp_atomic128_fetch_add(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size16, atomic_write)
}
// ADD

// SUB
void __swordomp_atomic8_fetch_sub(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size1, atomic_write)
}

void __swordomp_atomic16_fetch_sub(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size2, atomic_write)
}

void __swordomp_atomic32_fetch_sub(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size4, atomic_write)
}

void __swordomp_atomic64_fetch_sub(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size8, atomic_write)
}

void __swordomp_atomic128_fetch_sub(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size16, atomic_write)
}
// SUB

// COMPARE EXCHANGE
void __swordomp_atomic8_compare_exchange_val(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size1, atomic_write)
}

void __swordomp_atomic16_compare_exchange_val(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size2, atomic_write)
}

void __swordomp_atomic32_compare_exchange_val(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size4, atomic_write)
}

void __swordomp_atomic64_compare_exchange_val(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size8, atomic_write)
}

void __swordomp_atomic128_compare_exchange_val(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;	
	SAVE_ACCESS(access, size16, atomic_write)
}
// COMPARE EXCHANGE

// ATOMICS
