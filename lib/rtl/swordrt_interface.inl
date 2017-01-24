// UTIL

//__attribute__((destructor)) void __swordrt_finalize(void)
//{
//
//}

void __swordrt_init(uint64_t *access_min, uint64_t *access_max,
		uint64_t *function_min, uint64_t *function_max) {}

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
	SAVE_ACCESS(size1, unsafe_read)
}


void __swordomp_read2(void *addr, uint64_t hash) {
	SAVE_ACCESS(size2, unsafe_read)
}

void __swordomp_read4(void *addr, uint64_t hash) {
	SAVE_ACCESS(size4, unsafe_read)
}

void __swordomp_read8(void *addr, uint64_t hash) {
	SAVE_ACCESS(size8, unsafe_read)
}

void __swordomp_read16(void *addr, uint64_t hash) {
	SAVE_ACCESS(size16, unsafe_read)
}
// READS

// WRITES
void __swordomp_write1(void *addr, uint64_t hash) {
	SAVE_ACCESS(size1, unsafe_write)
}

void __swordomp_write2(void *addr, uint64_t hash) {
	SAVE_ACCESS(size2, unsafe_write)
}

void __swordomp_write4(void *addr, uint64_t hash) {
	SAVE_ACCESS(size4, unsafe_write)
}

void __swordomp_write8(void *addr, uint64_t hash) {
	SAVE_ACCESS(size8, unsafe_write)
}

void __swordomp_write16(void *addr, uint64_t hash) {
	SAVE_ACCESS(size16, unsafe_write)
}
// WRITES

// ATOMICS

// LOAD

void __swordomp_atomic8_load(void *addr, uint64_t hash) {
	// DEF_ACCESS

	// SAVE_ACCESS(atomic8_load, size1, atomic_read)
	SAVE_ACCESS(size1, atomic_read)
}

void __swordomp_atomic16_load(void *addr, uint64_t hash) {
	// DEF_ACCESS

	SAVE_ACCESS(size2, atomic_read)
}

void __swordomp_atomic32_load(void *addr, uint64_t hash) {
	SAVE_ACCESS(size4, atomic_read)
}

void __swordomp_atomic64_load(void *addr, uint64_t hash) {
	SAVE_ACCESS(size8, atomic_read)
}

void __swordomp_atomic128_load(void *addr, uint64_t hash) {
	SAVE_ACCESS(size16, atomic_read)
}
// LOAD

// STORE
void __swordomp_atomic8_store(void *addr, uint64_t hash) {
	SAVE_ACCESS(size1, atomic_write)
}

void __swordomp_atomic16_store(void *addr, uint64_t hash) {
	SAVE_ACCESS(size2, atomic_write)
}

void __swordomp_atomic32_store(void *addr, uint64_t hash) {
	SAVE_ACCESS(size4, atomic_write)
}

void __swordomp_atomic64_store(void *addr, uint64_t hash) {
	SAVE_ACCESS(size8, atomic_write)
}

void __swordomp_atomic128_store(void *addr, uint64_t hash) {
	SAVE_ACCESS(size16, atomic_write)
}
// STORE

// ADD
void __swordomp_atomic8_fetch_add(void *addr, uint64_t hash) {

}

void __swordomp_atomic16_fetch_add(void *addr, uint64_t hash) {

}

void __swordomp_atomic32_fetch_add(void *addr, uint64_t hash) {

}

void __swordomp_atomic64_fetch_add(void *addr, uint64_t hash) {

}

void __swordomp_atomic128_fetch_add(void *addr, uint64_t hash) {

}
// ADD

// SUB
void __swordomp_atomic8_fetch_sub(void *addr, uint64_t hash) {

}

void __swordomp_atomic16_fetch_sub(void *addr, uint64_t hash) {

}

void __swordomp_atomic32_fetch_sub(void *addr, uint64_t hash) {

}

void __swordomp_atomic64_fetch_sub(void *addr, uint64_t hash) {

}

void __swordomp_atomic128_fetch_sub(void *addr, uint64_t hash) {

}
// SUB

// COMPARE EXCHANGE
void __swordomp_atomic8_compare_exchange_val(void *addr, uint64_t hash) {

}

void __swordomp_atomic16_compare_exchange_val(void *addr, uint64_t hash) {

}

void __swordomp_atomic32_compare_exchange_val(void *addr, uint64_t hash) {

}

void __swordomp_atomic64_compare_exchange_val(void *addr, uint64_t hash) {

}

void __swordomp_atomic128_compare_exchange_val(void *addr, uint64_t hash) {

}

// COMPARE EXCHANGE

// ATOMICS
