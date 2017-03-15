// RUN: %raceomp-compile-and-run 2>&1 | FileCheck %s
#include <omp.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
  int var = 0;

  omp_nest_lock_t lock;
  omp_init_nest_lock(&lock);

  #pragma omp parallel num_threads(2) shared(var)
  {
    omp_set_nest_lock(&lock);
    omp_set_nest_lock(&lock);
    // Dummy locking.
    omp_unset_nest_lock(&lock);
    omp_unset_nest_lock(&lock);

    var++;
  }

  omp_destroy_nest_lock(&lock);

  int error = (var != 2);
  return error;
}

// CHECK: --------------------------------------------------
// CHECK: WARNING: SWORD: data race (program={{.*}})
// CHECK:   Two different threads made the following accesses:
// CHECK:     Write of size 4 in .omp_outlined. at {{.*}}lock-nested-unrelated.c:20:8
// CHECK:     Write of size 4 in .omp_outlined. at {{.*}}lock-nested-unrelated.c:20:8
// CHECK: --------------------------------------------------
