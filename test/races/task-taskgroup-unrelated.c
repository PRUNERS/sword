// RUN: %raceomp-compile-and-run 2>&1 | FileCheck %s
#include <omp.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
  int var = 0;

  #pragma omp parallel num_threads(2) shared(var)
  #pragma omp master
  {
    #pragma omp task shared(var)
    {
      var++;
      // Give master thread time to execute the task in the taskgroup.
      sleep(1);
    }

    #pragma omp taskgroup
    {
      #pragma omp task
      {
        // Dummy task.
      }

      // Give other threads time to steal the tasks.
      sleep(1);
    }

    var++;
  }

  int error = (var != 2);
  return error;
}

// XFAIL: *
