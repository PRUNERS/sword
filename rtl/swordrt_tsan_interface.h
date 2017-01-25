extern "C" {

thread_local int tid = 0;

void __tsan_init();
void __tsan_func_entry(void *pc);
void __tsan_func_exit();

void __tsan_read1(void *addr);
void __tsan_read2(void *addr);
void __tsan_read4(void *addr);
void __tsan_read8(void *addr);
void __tsan_read16(void *addr);

void __tsan_write1(void *addr);
void __tsan_write2(void *addr);
void __tsan_write4(void *addr);
void __tsan_write8(void *addr);
void __tsan_write16(void *addr);
}
