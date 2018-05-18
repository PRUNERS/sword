<div id="table-of-contents">
<h2>Table of Contents</h2>
<div id="text-table-of-contents">
<ul>
<li><a href="#orgce04724">1. License</a></li>
<li><a href="#orge8aedb5">2. Introduction</a></li>
<li><a href="#org9d0c661">3. Prerequisites</a></li>
<li><a href="#org9597d77">4. Installation</a>
<ul>
<li><a href="#org04bbf00">4.1. Manual Building</a>
<ul>
<li><a href="#orgc79f3e0">4.1.1. Stand-alone building</a></li>
</ul>
</li>
</ul>
</li>
<li><a href="#org3d95f56">5. Usage</a>
<ul>
<li><a href="#orgf4296dd">5.1. How to compile</a>
<ul>
<li><a href="#orgb87506d">5.1.1. Single source</a></li>
<li><a href="#orga02c06f">5.1.2. Makefile</a></li>
</ul>
</li>
<li><a href="#orgfa12680">5.2. Options</a></li>
<li><a href="#org48ebef1">5.3. Runtime Flags</a></li>
</ul>
</li>
<li><a href="#org4772775">6. Example</a></li>
<li><a href="#org9aa5c57">7. Contacts and Support</a></li>
<li><a href="#org10ffe3f">8. Members</a></li>
</ul>
</div>
</div>


<a id="orgce04724"></a>

# License

Please see LICENSE for usage terms.


<a id="orge8aedb5"></a>

# Introduction

<img src="resources/images/sword_logo.png" hspace="5" vspace="5" height="45%" width="45%" alt="Sword Logo" title="Sword" align="right" />

**Sword** is a data race detector for OpenMP programs.

Sword combines several techniques to identify data races in large
OpenMP applications, maintaining low runtime and zero overheads,
offering soundness and completeness of the data race detection
process. It builds on open-source tools infrastructure such as LLVM,
and OMPT to provide portability.


<a id="org9d0c661"></a>

# Prerequisites

To compile Sword you need a host Clang/LLVM version >= 6.0, a CMake
version >= 3.4.3, [GLPK](<https://www.gnu.org/software/glpk/>) version
>= 4.61, and [Boost](<https://www.boost.org/>) Libraries version >= 1.58.

Ninja build system is preferred. For more information how to obtain
Ninja visit <https://github.com/ninja-build/ninja>. (Note that this is
different than PRUNERS NINJA tool.)

Sword has been tested with the LLVM OpenMP Runtime version >= 6.0
(with OMPT support.


<a id="org9597d77"></a>

# Installation

Sword has been developed under LLVM 6.0 (for more information visit
<http://llvm.org>).


<a id="org04bbf00"></a>

## Manual Building

Sword comes as an LLVM tool, it can be compiled both as a stand-alone
tool or within the Clang/LLVM infrastructure.

In order to obtain and build Sword, follow the instructions below for
stand-alone or full Clang/LLVM with Sword support (instructions are
based on bash shell, Clang/LLVM 6.0 version, Ninja build system, and
the LLVM OpenMP Runtime with OMPT support).


<a id="orgc79f3e0"></a>

### Stand-alone building

Create a folder in which to download and build Sword:

    export SWORD_BUILD=$PWD/SwordBuild
    mkdir $SWORD_BUILD && cd $SWORD_BUILD

Obtain the LLVM OpenMP Runtime with OMPT support:

    git clone https://github.com/llvm-mirror/openmp.git openmp

and build it with the following command:

    export OPENMP_INSTALL=$HOME/usr           # or any other install path
    cd openmp/runtime
    mkdir build && cd build
    cmake -G Ninja \
     -D CMAKE_C_COMPILER=clang \
     -D CMAKE_CXX_COMPILER=clang++ \
     -D CMAKE_BUILD_TYPE=Release \
     -D CMAKE_INSTALL_PREFIX:PATH=$OPENMP_INSTALL \
     ..
    ninja -j8 -l8                             # or any number of available cores
    ninja install

Obtain Sword:

    cd $SWORD_BUILD
    git clone https://github.com/PRUNERS/sword.git sword

and build it with the following commands:

    export SWORD_INSTALL=$HOME/usr           # or any other install path
    cd sword
    mkdir build && cd build
    cmake -G Ninja \
     -D CMAKE_C_COMPILER=clang \
     -D CMAKE_CXX_COMPILER=clang++ \
     -D CMAKE_BUILD_TYPE=Release
     -D OMP_PREFIX:PATH=$OPENMP_INSTALL \
     -D CMAKE_INSTALL_PREFIX:PATH=${SWORD_INSTALL} \
     # -D GLPK_ROOT= \
     # -D BOOST_ROOT= \
     -D COMPRESSION=LZO .. \
     ninja -j8 -l8 # or any number of
     available cores ninja install cd ../..


<a id="org3d95f56"></a>

# Usage


<a id="orgf4296dd"></a>

## How to compile

Sword provides a command to compile your programs with Clang/LLVM
OpenMP and hide all the mechanisms necessary to detect data races.

The Sword compile command is called *clang-sword*, and this can be
used as a drop-in replacement of your compiler command (e.g., clang,
gcc, etc.).

The following are some of the examples of how one can integrate
*clang-sword* into her build system.


<a id="orgb87506d"></a>

### Single source

    clang-sword example.c -o example


<a id="orga02c06f"></a>

### Makefile

In your Makefile, set the following variables:

    CC=clang-sword


<a id="orgfa12680"></a>

## Options

The command *clang-sword* works as a compiler wrapper, all the
options available for clang are also available for *clang-sword*.


<a id="org48ebef1"></a>

## Runtime Flags

Runtime flags are passed via **SWORD&#95;OPTIONS** environment variable,
different flags are separated by spaces, e.g.:

    SWORD_OPTIONS="flush_shadow=1" ./myprogram

<table border="2" cellspacing="0" cellpadding="6" rules="groups" frame="hsides">


<colgroup>
<col  class="org-left" />

<col  class="org-left" />

<col  class="org-left" />
</colgroup>
<thead>
<tr>
<th scope="col" class="org-left">Flag Name</th>
<th scope="col" class="org-left">Default value</th>
<th scope="col" class="org-left">Description</th>
</tr>
</thead>

<tbody>
<tr>
<td class="org-left">traces&#95;path</td>
<td class="org-left">not set</td>
<td class="org-left">Specify the path where to save the data gathered by Sword at runtime.</td>
</tr>
</tbody>
</table>


<a id="org4772775"></a>

# Example

Let us take the program below and follow the steps to compile and
check the program for data races.

Suppose our program is called *myprogram.c*:

     1  #include <stdio.h>
     2  
     3  #define N 1000
     4  
     5  int main (int argc, char **argv)
     6  {
     7    int a[N];
     8  
     9  #pragma omp parallel for
    10    for (int i = 0; i < N - 1; i++) {
    11      a[i] = a[i + 1];
    12    }
    13  }

We compile the program as follow:

    clang-sword myprogram.c -o myprogram

Now we can run the program with the following commands:

    export OMP_NUM_THREADS=2
    ./myprogram

Sword will run the dynamic analysis and gather the data in the default
folder *sword<sub>data</sub>* in the current path. At the end of the execution
Sword will print out the commands we have to execute to run the offline
analysis, for this example it will print the following information:

    ################################################################
    
    SWORD data gathering terminated.
    
    To analyze the data and detect races, please execute:
    
    sword-offline-analysis --analysis-tool /path/to/sword-race-analysis --executable /path/to/your/executable --traces-path /path/to/sword_data --report-path /path/to/sword_report
    
    
    To print the results of theanalysis, please execute:
    
    sword-print-report --executable /path/to/your/executable --report-path /path/to/sword_report
    
    ################################################################

Now we can execute the offline analysis with the tool:

    sword-offline-analysis --analysis-tool sword-race-analysis --executable example --traces-path sword_data --report-path sword_report

Then, print the result of the analysis with:

    sword-print-report --executable example --report-path sword_report

For our example, the result will be the following:

    --------------------------------------------------
    WARNING: SWORD: data race (program=example)
      Two different threads made the following accesses:
        Write of size 4 in .omp_outlined._debug__ at /home/satzeni/work/compilers/sword/sword/build/example.c:11:10
        Read of size 4 in .omp_outlined._debug__ at /home/satzeni/work/compilers/sword/sword/build/example.c:11:12
    --------------------------------------------------


<a id="org9aa5c57"></a>

# Contacts and Support

-   [Slack Channel](https://pruners.slack.com)
    
    <ul style="list-style-type:circle"> <li> For an invitation please write an email to <a href="mailto:simone@cs.utah.edu?Subject=[sword-slack] Slack Invitation" target="_top">Simone Atzeni</a> with a reason why you want to be part of the PRUNERS Slack Team. </li> </ul>
-   E-Mail Contacts:
    
    <ul style="list-style-type:circle"> <li> <a href="mailto:simone@cs.utah.edu?Subject=[sword-dev]%20" target="_top">Simone Atzeni</a> </li> </ul>


<a id="org10ffe3f"></a>

# Members

<img src="resources/images/uofu_logo.png" hspace="15" vspace="5" height="23%" width="23%" alt="UofU Logo" title="University of Utah" style="float:left"/> <img src="resources/images/llnl_logo.png" hspace="70" vspace="5" height="30%" width="30%" alt="LLNL Logo" title="Lawrence Livermore National Laboratory" style="float:center" />

