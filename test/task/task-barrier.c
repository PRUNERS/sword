// RUN: %raceomp-compile-and-run 2>&1 | FileCheck %s
#include <omp.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
  int var = 0;

  #pragma omp parallel num_threads(2) shared(var)
  {
    #pragma omp master
    {
      #pragma omp task shared(var)
      {
        var++;
      }

      // Give other thread time to steal the task.
      sleep(1);
    }

    #pragma omp barrier

    #pragma omp master
    {
      var++;
    }
  }

  int error = (var != 2);
  return error;
}

// CHECK: SWORD did not find any race on '{{.*}}'.
