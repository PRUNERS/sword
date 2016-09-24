// READS
void __swordomp_read1(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size1, mutex_read);
	else
		swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size1, unsafe_read);
}

void __swordomp_read2(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size2, mutex_read);
	else
		swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size2, unsafe_read);
}

void __swordomp_read4(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size4, mutex_read);
	else
		swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size4, unsafe_read);
}

void __swordomp_read8(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size8, mutex_read);
	else
		swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size8, unsafe_read);
}
// READS

// WRITES
void __swordomp_write1(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size1, mutex_write);
	else
		swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size1, unsafe_write);
}

void __swordomp_write2(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size2, mutex_write);
	else
		swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size2, unsafe_write);
}

void __swordomp_write4(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size4, mutex_write);
	else
		swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size4, unsafe_write);
}

void __swordomp_write8(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size8, mutex_write);
	else
		swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size8, unsafe_write);
}
// WRITES

// ATOMICS

// LOAD
void __swordomp_atomic8_load(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size1, atomic_read);
}

void __swordomp_atomic16_load(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size2, atomic_read);
}

void __swordomp_atomic32_load(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size4, atomic_read);
}

void __swordomp_atomic64_load(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size8, atomic_read);
}

void __swordomp_atomic128_load(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size16, atomic_read);
}
// LOAD

// STORE
void __swordomp_atomic8_store(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size1, atomic_write);
}

void __swordomp_atomic16_store(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size2, atomic_write);
}

void __swordomp_atomic32_store(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size4, atomic_write);
}

void __swordomp_atomic64_store(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size8, atomic_write);
}

void __swordomp_atomic128_store(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size16, atomic_write);
}
// STORE

// ADD
void __swordomp_atomic8_fetch_add(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size1, atomic_write);
}

void __swordomp_atomic16_fetch_add(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size2, atomic_write);
}

void __swordomp_atomic32_fetch_add(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size4, atomic_write);
}

void __swordomp_atomic64_fetch_add(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size8, atomic_write);
}

void __swordomp_atomic128_fetch_add(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size16, atomic_write);
}
// ADD

// SUB
void __swordomp_atomic8_fetch_sub(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size1, atomic_write);
}

void __swordomp_atomic16_fetch_sub(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size2, atomic_write);
}

void __swordomp_atomic32_fetch_sub(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size4, atomic_write);
}

void __swordomp_atomic64_fetch_sub(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size8, atomic_write);
}

void __swordomp_atomic128_fetch_sub(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size16, atomic_write);
}
// SUB

// COMPARE EXCHANGE
void __swordomp_atomic8_compare_exchange_val(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size1, atomic_write);
}

void __swordomp_atomic16_compare_exchange_val(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size2, atomic_write);
}

void __swordomp_atomic32_compare_exchange_val(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size4, atomic_write);
}

void __swordomp_atomic64_compare_exchange_val(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size8, atomic_write);
}

void __swordomp_atomic128_compare_exchange_val(void *addr) {
	swordRT->CheckMemoryAccess((((size_t) addr) >> 3), CALLERPC, size16, atomic_write);
}
// COMPARE EXCHANGE

// ATOMICS