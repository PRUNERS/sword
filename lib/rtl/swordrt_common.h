//===-- swordrt_common.h ------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of Archer/SwordRT, an OpenMP race detector.
//===----------------------------------------------------------------------===//

#ifndef SWORDRT_COMMON_H
#define SWORDRT_COMMON_H

#include <cstdint>

#define TOTAL_ACCESSES			500000
#define NUM_OF_ACCESSES			100000
#define NUM_OF_CONFLICTS		100
#define THREAD_NUM				24

enum AccessSize {
	size1 = 0,
	size2,
	size4,
	size8,
	size16
};

enum AccessType {
	none = 0,
	unsafe_read,
	unsafe_write,
	atomic_read,
	atomic_write,
	mutex_read,
	mutex_write,
	nutex_read,
	nutex_write
};

struct AccessInfo
{
	uint64_t address;
	AccessSize size;
	AccessType type;
	size_t pc1;
	uint64_t cell1;
	uint64_t cell2;

	AccessInfo() {
		address = 0;
		size = size4;
		type = none;
		pc1 = 0;
		cell1 = 0;
		cell2 = 0;
	}

	AccessInfo(uint64_t a, AccessSize as, AccessType t, size_t p1, uint64_t c1, uint64_t c2) {
		address = a;
		size = as;
		type = t;
		pc1 = p1;
		cell1 = c1;
		cell1 = c2;
	}

	inline bool operator==(AccessInfo item) {
		if((item.address == address) &&
				(item.size == size) &&
				(item.type == type) &&
				(item.pc1 == pc1) &&
				(item.cell1 == cell1) &&
				(item.cell2 == cell2))
			return true;
		else
			return false;
	}

	inline bool operator!=(AccessInfo item) {
		if(item.address != address)
			return true;
		else
			return false;
	}

};

class ByAddress
{
public:
	ByAddress(uint64_t address) : address(address) {}
	bool operator() (const AccessInfo &access) const { return access.address == address; }
private:
	const uint64_t address;
};

void malloc_host();
void malloc_device();
void set_device();
void copy_to_device(int chunk);

#endif  // SWORDRT_COMMON_H
