// RUN: %raceomp-compile-and-run 2>&1 | FileCheck %s
#include <stdlib.h>

int main (int argc, char* argv[])
{
  int len=1000;
  int i,j;
  if (argc>1)
    len = atoi(argv[1]);
  int a[len];
  a[0] = 2;

#pragma omp parallel
  {
    for (j=0; j<2; j++)
#pragma omp for
      for (i=0;i<len;i++)
        a[i]=a[i]+a[0];
  }
  return 0;
}

// CHECK: --------------------------------------------------
// CHECK: WARNING: SWORD: data race (program={{.*}})
// CHECK:   Two different threads made the following accesses:
// CHECK:     Write of size 4 in .omp_outlined. at {{.*}}critical-unrelated.c:16:8
// CHECK:     Write of size 4 in .omp_outlined. at {{.*}}critical-unrelated.c:16:8
// CHECK: --------------------------------------------------
