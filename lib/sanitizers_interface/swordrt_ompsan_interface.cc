#include "swordrt_ompsan_interface.h"

void __attribute__((weak)) __attribute__((weak)) __ompsan_init() {}

void __attribute__((weak)) __ompsan_read1(void *addr, uint64_t hash1, bool *conflict, uint64_t *hash2) {}
void __attribute__((weak)) __ompsan_read2(void *addr, uint64_t hash1, bool *conflict, uint64_t *hash2) {}
void __attribute__((weak)) __ompsan_read4(void *addr, uint64_t hash1, bool *conflict, uint64_t *hash2) {}
void __attribute__((weak)) __ompsan_read8(void *addr, uint64_t hash1, bool *conflict, uint64_t *hash2) {}
void __attribute__((weak)) __ompsan_read16(void *addr, uint64_t hash1, bool *conflict, uint64_t *hash2) {}

void __attribute__((weak)) __ompsan_write1(void *addr, uint64_t hash1, bool *conflict, uint64_t *hash2) {}
void __attribute__((weak)) __ompsan_write2(void *addr, uint64_t hash1, bool *conflict, uint64_t *hash2) {}
void __attribute__((weak)) __ompsan_write4(void *addr, uint64_t hash1, bool *conflict, uint64_t *hash2) {}
void __attribute__((weak)) __ompsan_write8(void *addr, uint64_t hash1, bool *conflict, uint64_t *hash2) {}
void __attribute__((weak)) __ompsan_write16(void *addr, uint64_t hash1, bool *conflict, uint64_t *hash2) {}
