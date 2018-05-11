// RUN: %libsword-compile-and-run-race 2>&1 | FileCheck %s
#include <omp.h>
#include <stdio.h>

#define N 100

int main(int argc, char* argv[])
{
	int var[N];

#pragma omp parallel for num_threads(2) shared(var)
	for(int i = 0; i < N; i++) {
		var[i] = i;
	}

#pragma omp parallel for num_threads(2) shared(var)
	for(int i = 1; i < N; i++) {
#pragma omp critical
		{
			var[i] = var[i - 1];
		}
	}
}

// CHECK: SWORD did not find any race on '{{.*}}'.
