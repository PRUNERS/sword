// RUN: %raceomp-compile-and-run 2>&1 | FileCheck %s
#include <stdlib.h>

#define N 10

int main (int argc, char* argv[])
{
	int i,j;
	int a[N];
	a[0] = 2;

#pragma omp parallel for num_threads(2)
	{
          for (i = 0;i < N; i++) {
            a[i] = a[i] + a[0];
          }
	}

	return 0;
}

// CHECK: --------------------------------------------------
// CHECK: WARNING: SWORD: data race (program={{.*}})
// CHECK:   Two different threads made the following accesses:
// CHECK:     Read of size 4 in .omp_outlined. at {{.*}}parallel-constant.c:15:27
// CHECK:     Write of size 4 in .omp_outlined. at {{.*}}parallel-constant.c:15:18
// CHECK: --------------------------------------------------
