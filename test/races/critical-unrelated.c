// RUN: %raceomp-compile-and-run 2>&1 | FileCheck %s
#include <omp.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
  int var = 0;

  #pragma omp parallel num_threads(2) shared(var)
  {
    #pragma omp critical
    {
      // Dummy region.
    }

    var++;
  }
}

// CHECK: --------------------------------------------------
// CHECK: WARNING: SWORD: data race (program={{.*}})
// CHECK:   Two different threads made the following accesses:
// CHECK:     Write of size 4 in .omp_outlined. at {{.*}}critical-unrelated.c:16:8
// CHECK:     Write of size 4 in .omp_outlined. at {{.*}}critical-unrelated.c:16:8
// CHECK: --------------------------------------------------
