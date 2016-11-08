
extern void __tsan_init();
extern void __tsan_func_entry(void *pc);
extern void __tsan_func_exit();

extern void __tsan_read1(void *addr);
extern void __tsan_read2(void *addr);
extern void __tsan_read4(void *addr);
extern void __tsan_read8(void *addr);

extern void __tsan_write1(void *addr);
extern void __tsan_write2(void *addr);
extern void __tsan_write4(void *addr);
extern void __tsan_write8(void *addr);
