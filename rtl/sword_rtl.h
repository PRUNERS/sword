//===-- sword_rtl.h ------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of Sword/Sword, an OpenMP race detector.
//===----------------------------------------------------------------------===//

#ifndef SWORD_RTL_H
#define SWORD_RTL_H

#include "sword_common.h"

#include <fcntl.h>
#include <sys/stat.h>

#include <future>
#include <vector>

#define ALWAYS_INLINE			__attribute__((always_inline))
#define CALLERPC 				((size_t) __builtin_return_address(0))

// Global Variables
std::atomic<ompt_id_t> current_parallel_idx(0);

// Thread Local Variables
thread_local unsigned char __LZO_MMODEL *out;
thread_local int tid = 0;
thread_local int __sword_status__ = 0;
thread_local TraceItem *accesses;
thread_local TraceItem *accesses1;
thread_local TraceItem *accesses2;
thread_local uint64_t idx = 0;
thread_local ompt_id_t parallel_idx = 0;
thread_local FILE *datafile = NULL;
thread_local char *buffer = NULL;
thread_local std::future<bool> fut;
thread_local size_t offset = 0;

#endif  // SWORD_RTL_H
