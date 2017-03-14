// RUN: %raceomp-compile-and-run | FileCheck %s
#include <omp.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
  int var = 0;

  #pragma omp parallel num_threads(2) shared(var)
  {
    var++;
  }

  int error = (var != 2);
  fprintf(stderr, "DONE\n");
  return error;
}

// CHECK: WARNING: SWORD: data race
// CHECK:   Two different threads made the following accesses:
// CHECK: Write of size 4 in .omp_outlined. at {{.*}}parallel-simple.c:11:8
// CHECK: Write of size 4 in .omp_outlined. at {{.*}}parallel-simple.c:11:8
// CHECK: DONE
