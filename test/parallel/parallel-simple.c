// RUN: %libsword-compile-and-run-race 2>&1 | FileCheck %s
#include <omp.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
  int var = 0;

  #pragma omp parallel num_threads(2) shared(var)
  {
    if (omp_get_thread_num() == 1) {
      var++;
    }
  } // implicit barrier

  var++;

  int error = (var != 2);
  return error;
}

// CHECK: SWORD did not find any race on '{{.*}}'.
