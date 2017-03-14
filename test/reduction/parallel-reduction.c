// RUN: %raceomp-compile-and-run 2>&1 | FileCheck %s
#include <omp.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
  int var = 0;

  // Number of threads is empirical: We need enough threads so that
  // the reduction is really performed hierarchically in the barrier!
  #pragma omp parallel num_threads(5) reduction(+: var)
  {
    var = 1;
  }

  int error = (var != 5);
  return error;
}

// CHECK: SWORD did not find any race on '{{.*}}'.
