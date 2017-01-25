// UTIL

//__attribute__((destructor)) void __sword_finalize(void)
//{
//
//}

int __sword_get_status() {
	return __sword_status__;
}

void __sword_status_inc() {
	__sword_status__++;
}

void __sword_status_dec() {
	__sword_status__++;
}

void __sword_func_entry(void *pc) {}

void __sword_func_exit() {}

// UTIL

// READS
void __sword_read1(void *addr) {
	SAVE_ACCESS(size1, unsafe_read)
}


void __sword_read2(void *addr) {
	SAVE_ACCESS(size2, unsafe_read)
}

void __sword_read4(void *addr) {
	SAVE_ACCESS(size4, unsafe_read)
}

void __sword_read8(void *addr) {
	SAVE_ACCESS(size8, unsafe_read)
}

void __sword_read16(void *addr) {
	SAVE_ACCESS(size16, unsafe_read)
}
// READS

// WRITES
void __sword_write1(void *addr) {
	SAVE_ACCESS(size1, unsafe_write)
}

void __sword_write2(void *addr) {
	SAVE_ACCESS(size2, unsafe_write)
}

void __sword_write4(void *addr) {
	SAVE_ACCESS(size4, unsafe_write)
}

void __sword_write8(void *addr) {
	SAVE_ACCESS(size8, unsafe_write)
}

void __sword_write16(void *addr) {
	SAVE_ACCESS(size16, unsafe_write)
}
// WRITES

// ATOMICS

// LOAD

void __sword_atomic8_load(void *addr) {
	// DEF_ACCESS

	// SAVE_ACCESS(atomic8_load, size1, atomic_read)
	SAVE_ACCESS(size1, atomic_read)
}

void __sword_atomic16_load(void *addr) {
	// DEF_ACCESS

	SAVE_ACCESS(size2, atomic_read)
}

void __sword_atomic32_load(void *addr) {
	SAVE_ACCESS(size4, atomic_read)
}

void __sword_atomic64_load(void *addr) {
	SAVE_ACCESS(size8, atomic_read)
}

void __sword_atomic128_load(void *addr) {
	SAVE_ACCESS(size16, atomic_read)
}
// LOAD

// STORE
void __sword_atomic8_store(void *addr) {
	SAVE_ACCESS(size1, atomic_write)
}

void __sword_atomic16_store(void *addr) {
	SAVE_ACCESS(size2, atomic_write)
}

void __sword_atomic32_store(void *addr) {
	SAVE_ACCESS(size4, atomic_write)
}

void __sword_atomic64_store(void *addr) {
	SAVE_ACCESS(size8, atomic_write)
}

void __sword_atomic128_store(void *addr) {
	SAVE_ACCESS(size16, atomic_write)
}
// STORE

// ADD
void __sword_atomic8_fetch_add(void *addr) {

}

void __sword_atomic16_fetch_add(void *addr) {

}

void __sword_atomic32_fetch_add(void *addr) {

}

void __sword_atomic64_fetch_add(void *addr) {

}

void __sword_atomic128_fetch_add(void *addr) {

}
// ADD

// SUB
void __sword_atomic8_fetch_sub(void *addr) {

}

void __sword_atomic16_fetch_sub(void *addr) {

}

void __sword_atomic32_fetch_sub(void *addr) {

}

void __sword_atomic64_fetch_sub(void *addr) {

}

void __sword_atomic128_fetch_sub(void *addr) {

}
// SUB

// COMPARE EXCHANGE
void __sword_atomic8_compare_exchange_val(void *addr) {

}

void __sword_atomic16_compare_exchange_val(void *addr) {

}

void __sword_atomic32_compare_exchange_val(void *addr) {

}

void __sword_atomic64_compare_exchange_val(void *addr) {

}

void __sword_atomic128_compare_exchange_val(void *addr) {

}

// COMPARE EXCHANGE

// ATOMICS
