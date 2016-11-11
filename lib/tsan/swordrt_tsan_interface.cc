#include "swordrt_tsan_interface.h"

void __attribute__((weak)) __tsan_init() {}
void __attribute__((weak)) __tsan_func_entry(void *pc) {}
void __attribute__((weak)) __tsan_func_exit() {}

void __attribute__((weak)) __tsan_read1(void *addr) {}
void __attribute__((weak)) __tsan_read2(void *addr) {}
void __attribute__((weak)) __tsan_read4(void *addr) {}
void __attribute__((weak)) __tsan_read8(void *addr) {}

void __attribute__((weak)) __tsan_write1(void *addr) {}
void __attribute__((weak)) __tsan_write2(void *addr) {}
void __attribute__((weak)) __tsan_write4(void *addr) {}
void __attribute__((weak)) __tsan_write8(void *addr) {}
