// RUN: %raceomp-compile-and-run 2>&1 | FileCheck %s
#include <omp.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
  int a,b,c = 0;

#pragma omp parallel num_threads(2)
#pragma omp master
  {
    a++;
#pragma omp task shared(a,b)
    {
      a++;
      b++;
    }
#pragma omp task shared(b,c)
    {
      b++;
      c++;
    }
    c++;
    sleep(1);
  }

  return 0;
}

// XFAIL: *
