int __swordomp_get_status() {
	return __swordomp_status__;
}

void __swordomp_status_inc() {
	__swordomp_status__++;
}

void __swordomp_status_dec() {
	__swordomp_status__--;
}

// READS
void __swordomp_read1(void *addr, uint64_t hash) {

}

void __swordomp_read2(void *addr, uint64_t hash) {

}

void __swordomp_read4(void *addr, uint64_t hash) {

}

void __swordomp_read8(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;
	if((access >= (size_t) stack) &&
			(access < (size_t) stack + stacksize))
		return;

	// binAccesses = insert(binAccesses, hash, access, 1, size8, unsafe_read);
	std::unordered_map<uint64_t, node>::const_iterator item = binAccesses.find(hash);
	if(item == binAccesses.end()) {
		binAccesses.insert(std::make_pair(hash, node(access, 1, size8, unsafe_read)));
	} else {
		if(access < item->second.address) {
			binAccesses[hash].address = access;
			binAccesses[hash].count++;
		} else if(((access - item->second.address) / size8) > item->second.count) {
			binAccesses[hash].count++;
		}
	}
}
// READS

// WRITES
void __swordomp_write1(void *addr, uint64_t hash) {

}

void __swordomp_write2(void *addr, uint64_t hash) {

}

void __swordomp_write4(void *addr, uint64_t hash) {

}

void __swordomp_write8(void *addr, uint64_t hash) {
	size_t access = (size_t) addr;
	if((access >= (size_t) stack) &&
			(access < (size_t) stack + stacksize))
		return;

	// binAccesses = insert(binAccesses, hash, access, 1, size8, unsafe_write);
	std::unordered_map<uint64_t, node>::const_iterator item = binAccesses.find(hash);
	if(item == binAccesses.end()) {
		binAccesses.insert(std::make_pair(hash, node(access, 1, size8, unsafe_write)));
	} else {
		if(access < item->second.address) {
			binAccesses[hash].address = access;
			binAccesses[hash].count++;
		} else if(((access - item->second.address) / size8) > item->second.count) {
			binAccesses[hash].count++;
		}
	}
}
// WRITES

// ATOMICS

// LOAD
void __swordomp_atomic8_load(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size1, atomic_read);
}

void __swordomp_atomic16_load(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size2, atomic_read);
}

void __swordomp_atomic32_load(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size4, atomic_read);
}

void __swordomp_atomic64_load(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size8, atomic_read);
}

void __swordomp_atomic128_load(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size16, atomic_read);
}
// LOAD

// STORE
void __swordomp_atomic8_store(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size1, atomic_write);
}

void __swordomp_atomic16_store(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size2, atomic_write);
}

void __swordomp_atomic32_store(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size4, atomic_write);
}

void __swordomp_atomic64_store(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size8, atomic_write);
}

void __swordomp_atomic128_store(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size16, atomic_write);
}
// STORE

// ADD
void __swordomp_atomic8_fetch_add(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size1, atomic_write);
}

void __swordomp_atomic16_fetch_add(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size2, atomic_write);
}

void __swordomp_atomic32_fetch_add(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size4, atomic_write);
}

void __swordomp_atomic64_fetch_add(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size8, atomic_write);
}

void __swordomp_atomic128_fetch_add(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size16, atomic_write);
}
// ADD

// SUB
void __swordomp_atomic8_fetch_sub(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size1, atomic_write);
}

void __swordomp_atomic16_fetch_sub(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size2, atomic_write);
}

void __swordomp_atomic32_fetch_sub(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size4, atomic_write);
}

void __swordomp_atomic64_fetch_sub(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size8, atomic_write);
}

void __swordomp_atomic128_fetch_sub(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size16, atomic_write);
}
// SUB

// COMPARE EXCHANGE
void __swordomp_atomic8_compare_exchange_val(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size1, atomic_write);
}

void __swordomp_atomic16_compare_exchange_val(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size2, atomic_write);
}

void __swordomp_atomic32_compare_exchange_val(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size4, atomic_write);
}

void __swordomp_atomic64_compare_exchange_val(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size8, atomic_write);
}

void __swordomp_atomic128_compare_exchange_val(void *addr, uint64_t hash) {
	swordRT->CheckMemoryAccess((((size_t) addr)), CALLERPC, size16, atomic_write);
}
// COMPARE EXCHANGE

// ATOMICS
