#include <stdint.h>

typedef unsigned long long uptr;

typedef enum {
  mo_relaxed,
  mo_consume,
  mo_acquire,
  mo_release,
  mo_acq_rel,
  mo_seq_cst
} morder;

typedef unsigned char u8;
typedef unsigned short u16;  // NOLINT
typedef unsigned int u32;
typedef unsigned long long u64;  // NOLINT
typedef signed   char s8;
typedef signed   short s16;  // NOLINT
typedef signed   int s32;
typedef signed   long long s64;  // NOLINT
typedef unsigned char      a8;
typedef unsigned short     a16;  // NOLINT
typedef unsigned int       a32;
typedef unsigned long long a64;  // NOLINT

extern "C" {
void __ompsan_init();

void __ompsan_read1(void *addr, uint64_t pc1, bool *conflict, uint64_t *pc2);
void __ompsan_read2(void *addr, uint64_t pc1, bool *conflict, uint64_t *pc2);
void __ompsan_read4(void *addr, uint64_t pc1, bool *conflict, uint64_t *pc2);
void __ompsan_read8(void *addr, uint64_t pc1, bool *conflict, uint64_t *pc2);
void __ompsan_read16(void *addr, uint64_t pc1, bool *conflict, uint64_t *pc2);

void __ompsan_write1(void *addr, uint64_t pc1, bool *conflict, uint64_t *pc2);
void __ompsan_write2(void *addr, uint64_t pc1, bool *conflict, uint64_t *pc2);
void __ompsan_write4(void *addr, uint64_t pc1, bool *conflict, uint64_t *pc2);
void __ompsan_write8(void *addr, uint64_t pc1, bool *conflict, uint64_t *pc2);
void __ompsan_write16(void *addr, uint64_t pc1, bool *conflict, uint64_t *pc2);

a8 __ompsan_atomic8_load(const volatile a8 *a, morder mo);
a16 __ompsan_atomic16_load(const volatile a16 *a, morder mo);
a32 __ompsan_atomic32_load(const volatile a32 *a, morder mo);
a64 __ompsan_atomic64_load(const volatile a64 *a, morder mo);

void __ompsan_atomic8_store(volatile a8 *a, a8 v, morder mo);
void __ompsan_atomic16_store(volatile a16 *a, a16 v, morder mo);
void __ompsan_atomic32_store(volatile a32 *a, a32 v, morder mo);
void __ompsan_atomic64_store(volatile a64 *a, a64 v, morder mo);

a8 __ompsan_atomic8_exchange(volatile a8 *a, a8 v, morder mo);
a16 __ompsan_atomic16_exchange(volatile a16 *a, a16 v, morder mo);
a32 __ompsan_atomic32_exchange(volatile a32 *a, a32 v, morder mo);
a64 __ompsan_atomic64_exchange(volatile a64 *a, a64 v, morder mo);

a8 __ompsan_atomic8_fetch_add(volatile a8 *a, a8 v, morder mo);
a16 __ompsan_atomic16_fetch_add(volatile a16 *a, a16 v, morder mo);
a32 __ompsan_atomic32_fetch_add(volatile a32 *a, a32 v, morder mo);
a64 __ompsan_atomic64_fetch_add(volatile a64 *a, a64 v, morder mo);

a8 __ompsan_atomic8_fetch_sub(volatile a8 *a, a8 v, morder mo);
a16 __ompsan_atomic16_fetch_sub(volatile a16 *a, a16 v, morder mo);
a32 __ompsan_atomic32_fetch_sub(volatile a32 *a, a32 v, morder mo);
a64 __ompsan_atomic64_fetch_sub(volatile a64 *a, a64 v, morder mo);

a8 __ompsan_atomic8_fetch_and(volatile a8 *a, a8 v, morder mo);
a16 __ompsan_atomic16_fetch_and(volatile a16 *a, a16 v, morder mo);
a32 __ompsan_atomic32_fetch_and(volatile a32 *a, a32 v, morder mo);
a64 __ompsan_atomic64_fetch_and(volatile a64 *a, a64 v, morder mo);

a8 __ompsan_atomic8_fetch_or(volatile a8 *a, a8 v, morder mo);
a16 __ompsan_atomic16_fetch_or(volatile a16 *a, a16 v, morder mo);
a32 __ompsan_atomic32_fetch_or(volatile a32 *a, a32 v, morder mo);
a64 __ompsan_atomic64_fetch_or(volatile a64 *a, a64 v, morder mo);

a8 __ompsan_atomic8_fetch_xor(volatile a8 *a, a8 v, morder mo);
a16 __ompsan_atomic16_fetch_xor(volatile a16 *a, a16 v, morder mo);
a32 __ompsan_atomic32_fetch_xor(volatile a32 *a, a32 v, morder mo);
a64 __ompsan_atomic64_fetch_xor(volatile a64 *a, a64 v, morder mo);

a8 __ompsan_atomic8_fetch_nand(volatile a8 *a, a8 v, morder mo);
a16 __ompsan_atomic16_fetch_nand(volatile a16 *a, a16 v, morder mo);
a32 __ompsan_atomic32_fetch_nand(volatile a32 *a, a32 v, morder mo);
a64 __ompsan_atomic64_fetch_nand(volatile a64 *a, a64 v, morder mo);

int __ompsan_atomic8_compare_exchange_strong(volatile a8 *a, a8 *c, a8 v,
                                           morder mo, morder fmo);
int __ompsan_atomic16_compare_exchange_strong(volatile a16 *a, a16 *c, a16 v,
                                            morder mo, morder fmo);
int __ompsan_atomic32_compare_exchange_strong(volatile a32 *a, a32 *c, a32 v,
                                            morder mo, morder fmo);
int __ompsan_atomic64_compare_exchange_strong(volatile a64 *a, a64 *c, a64 v,
                                            morder mo, morder fmo);

int __ompsan_atomic8_compare_exchange_weak(volatile a8 *a, a8 *c, a8 v, morder mo,
                                         morder fmo);
int __ompsan_atomic16_compare_exchange_weak(volatile a16 *a, a16 *c, a16 v,
                                          morder mo, morder fmo);
int __ompsan_atomic32_compare_exchange_weak(volatile a32 *a, a32 *c, a32 v,
                                          morder mo, morder fmo);
int __ompsan_atomic64_compare_exchange_weak(volatile a64 *a, a64 *c, a64 v,
                                          morder mo, morder fmo);

a8 __ompsan_atomic8_compare_exchange_val(volatile a8 *a, a8 c, a8 v, morder mo,
                                       morder fmo);
a16 __ompsan_atomic16_compare_exchange_val(volatile a16 *a, a16 c, a16 v,
                                         morder mo, morder fmo);
a32 __ompsan_atomic32_compare_exchange_val(volatile a32 *a, a32 c, a32 v,
                                         morder mo, morder fmo);
a64 __ompsan_atomic64_compare_exchange_val(volatile a64 *a, a64 c, a64 v,
                                         morder mo, morder fmo);

void __ompsan_atomic_thread_fence(morder mo);
void __ompsan_atomic_signal_fence(morder mo);

void __ompsan_go_atomic32_load(uptr cpc, uptr pc, u8 *a);
void __ompsan_go_atomic64_load(uptr cpc, uptr pc, u8 *a);
void __ompsan_go_atomic32_store(uptr cpc, uptr pc, u8 *a);
void __ompsan_go_atomic64_store(uptr cpc, uptr pc, u8 *a);
void __ompsan_go_atomic32_fetch_add(uptr cpc, uptr pc, u8 *a);
void __ompsan_go_atomic64_fetch_add(uptr cpc, uptr pc, u8 *a);
void __ompsan_go_atomic32_exchange(uptr cpc, uptr pc, u8 *a);
void __ompsan_go_atomic64_exchange(uptr cpc, uptr pc, u8 *a);
void __ompsan_go_atomic32_compare_exchange(uptr cpc, uptr pc, u8 *a);
void __ompsan_go_atomic64_compare_exchange(uptr cpc, uptr pc, u8 *a);
}
