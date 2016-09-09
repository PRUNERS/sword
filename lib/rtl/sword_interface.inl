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
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_read, "READ0");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_read, "READ0");
}

void __swordomp_read41(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_read, "READ1");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_read, "READ1");
}

void __swordomp_read42(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_read, "READ2");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_read, "READ2");
}

void __swordomp_read43(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_read, "READ3");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_read, "READ3");
}

void __swordomp_read44(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_read, "READ4");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_read, "READ4");
}

void __swordomp_read45(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_read, "READ5");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_read, "READ5");
}

void __swordomp_read46(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_read, "READ6");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_read, "READ6");
}

void __swordomp_read47(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_read, "READ7");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_read, "READ7");
}

void __swordomp_read48(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_read, "READ8");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_read, "READ8");
}

void __swordomp_read49(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_read, "READ9");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_read, "READ9");
}

void __swordomp_read50(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_read, "READ50");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_read, "READ50");
}

void __swordomp_read51(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_read, "READ51");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_read, "READ51");
}

void __swordomp_read52(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_read, "READ52");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_read, "READ52");
}

void __swordomp_read53(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_read, "READ53");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_read, "READ53");
}

void __swordomp_read54(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_read, "READ54");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_read, "READ54");
}

void __swordomp_read55(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_read, "READ55");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_read, "READ55");
}

void __swordomp_read8(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size8, mutex_read, "READ8");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size8, unsafe_read, "READ8");
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
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_write, "WRITE");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_write, "WRITE");
}

void __swordomp_write41(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_write, "WRITE1");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_write, "WRITE1");
}

void __swordomp_write42(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_write, "WRITE2");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_write, "WRITE2");
}

void __swordomp_write43(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_write, "WRITE3");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_write, "WRITE3");
}

void __swordomp_write44(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_write, "WRITE4");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_write, "WRITE4");
}

void __swordomp_write45(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_write, "WRITE5");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_write, "WRITE5");
}

void __swordomp_write46(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_write, "WRITE6");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_write, "WRITE6");
}

void __swordomp_write47(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_write, "WRITE7");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_write, "WRITE7");
}

void __swordomp_write48(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_write, "WRITE8");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_write, "WRITE8");
}

void __swordomp_write49(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_write, "WRITE9");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_write, "WRITE9");
}

void __swordomp_write50(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_write, "WRITE50");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_write, "WRITE50");
}

void __swordomp_write51(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_write, "WRITE51");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_write, "WRITE51");
}

void __swordomp_write52(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_write, "WRITE52");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_write, "WRITE52");
}

void __swordomp_write53(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_write, "WRITE53");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_write, "WRITE53");
}

void __swordomp_write54(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_write, "WRITE54");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_write, "WRITE54");
}

void __swordomp_write55(void *addr) {
	if(__swordomp_is_critical__)
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, mutex_write, "WRITE55");
	else
		swordRT->CheckMemoryAccess((uint64_t) addr, CALLERPC, size4, unsafe_write, "WRITE55");
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