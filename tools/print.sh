#!/bin/bash

sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/c_mandel.par.clang-sword --report-path sword_races/c_mandel.par.24 2> sword_races/c_mandel.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/c_pi.par.clang-sword --report-path sword_races/c_pi.par.24 2> sword_races/c_pi.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/c_testPath.par.clang-sword --report-path sword_races/c_testPath.par.24 2> sword_races/c_testPath.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/c_loopA.badSolution.par.clang-sword --report-path sword_races/c_loopA.badSolution.par.24 2> sword_races/c_loopA.badSolution.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/c_loopB.badSolution2.par.clang-sword --report-path sword_races/c_loopB.badSolution2.par.24 2> sword_races/c_loopB.badSolution2.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/c_loopB.badSolution1.par.clang-sword --report-path sword_races/c_loopB.badSolution1.par.24 2> sword_races/c_loopB.badSolution1.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/c_loopA.solution2.par.clang-sword --report-path sword_races/c_loopA.solution2.par.24 2> sword_races/c_loopA.solution2.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/c_loopB.pipelineSolution.par.clang-sword --report-path sword_races/c_loopB.pipelineSolution.par.24 2> sword_races/c_loopB.pipelineSolution.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/c_loopA.solution3.par.clang-sword --report-path sword_races/c_loopA.solution3.par.24 2> sword_races/c_loopA.solution3.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/c_loopA.solution1.par.clang-sword --report-path sword_races/c_loopA.solution1.par.24 2> sword_races/c_loopA.solution1.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/c_jacobi01.par.clang-sword --report-path sword_races/c_jacobi01.par.24 2> sword_races/c_jacobi01.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/c_jacobi02.par.clang-sword --report-path sword_races/c_jacobi02.par.24 2> sword_races/c_jacobi02.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/c_lu.par.clang-sword --report-path sword_races/c_lu.par.24 2> sword_races/c_lu.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/c_md.par.clang-sword --report-path sword_races/c_md.par.24 2> sword_races/c_md.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/c_fft6.par.clang-sword --report-path sword_races/c_fft6.par.24 2> sword_races/c_fft6.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/c_qsort.par.clang-sword --report-path sword_races/c_qsort.par.24 2> sword_races/c_qsort.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/cpp_qsomp5.par.clang-sword --report-path sword_races/cpp_qsomp5.par.24 2> sword_races/cpp_qsomp5.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/cpp_qsomp6.par.clang-sword --report-path sword_races/cpp_qsomp6.par.24 2> sword_races/cpp_qsomp6.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/cpp_qsomp2.par.clang-sword --report-path sword_races/cpp_qsomp2.par.24 2> sword_races/cpp_qsomp2.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/cpp_qsomp1.par.clang-sword --report-path sword_races/cpp_qsomp1.par.24 2> sword_races/cpp_qsomp1.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/OmpSCR_v2.0/bin/c_fft.par.clang-sword --report-path sword_races/c_fft.par.24 2> sword_races/c_fft.par.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/miniFE-2.0.1/miniFE_openmp_opt/miniFE.x.clang-sword --report-path sword_races/miniFE.x.24 2> sword_races/miniFE.x.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/HPCCG-1.0/test_HPCCG.clang-sword --report-path sword_races/test_HPCCG.24 2> sword_races/test_HPCCG.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/lulesh2.0.3/lulesh2.0.24.clang-sword --report-path sword_races/lulesh2.0.24 2> sword_races/test_HPCCG.24/races.rep
sword-print-report --executable $HOME/pruners/sword_evaluation/benchmarks/AMG2013/test/amg2013.clang-sword --report-path sword_races/amg2013.24 2> sword_races/amg2013.24/races.rep
