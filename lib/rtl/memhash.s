#
# Upcaste Performance Libraries
# Copyright (C) 2012-2013 Jesse W. Towner
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

#
# uint64_t memhash(void const* key, size_t n, uint64_t seed);
#
# Based off of the fast byteswap hash function by Steven Fuerst.
# http://locklessinc.com/articles/fast_hash/ 
#

.text

.align 16, 0x90
factor128:
.octa 0xd6c573e9c613993d5a379ab38dc5a46b

.global memhash
.type memhash, @function
memhash:

    movabs      $0x1591aefa5e7e5a17, %r8
    movq        %rsi, %xmm0
    movq        %rdx, %xmm1
    movabs      $0x2bb6863566c4e761, %r9
    movdqa      factor128(%rip), %xmm2
    cmp         $0x20, %rsi
    jb          2f
    mov         %rsi, %rax
    shr         $5, %rax
    testb       $0xf, %dil
    jne         5f

    # Aligned main-loop, process 32-byte aligned chunks at a time
    # using 16-byte signed multiplication and unpack to mix.

.align 8, 0x90
1:  paddq       (%rdi), %xmm0
    paddq       0x10(%rdi), %xmm1
    pmullw      %xmm2, %xmm0
    pmullw      %xmm2, %xmm1
    add         $0x20, %rdi
    movdqa      %xmm0, %xmm3
    punpckhbw   %xmm1, %xmm0
    punpcklbw   %xmm3, %xmm1
    dec         %rax
    jne         1b
    pxor        %xmm1, %xmm0

    # Check for remaining chunk, if not extract 128 bits state and
    # proceed to final mix.

2:  testb       $0x1f, %sil
    jne         4f
    movhlps     %xmm0, %xmm1
    movq        %xmm0, %rax
    movq        %xmm1, %rcx

    # Use 3x 128bit multiply and xorshift for final mix.

3:  imul        %r9, %rcx
    mul         %r8
    add         %rdx, %rcx
    xor         %rcx, %rax
    imul        %r9, %rcx
    mul         %r8
    add         %rdx, %rcx
    xor         %rcx, %rax
    imul        %r9, %rcx
    mul         %r8
    add         %rdx, %rcx
    xor         %rcx, %rax
    retq

    # Process remaining chunk in piece-wise manner, up to 31-bytes in size.

4:  testb       $0x10, %sil
    je          4f
    movdqu      (%rdi), %xmm3
    pxor        %xmm3, %xmm0
    add         $0x10, %rdi
    pmullw      %xmm2, %xmm0
4:  xor         %rdx, %rdx
    movhlps     %xmm0, %xmm1
    movq        %xmm0, %rax
    movq        %xmm1, %rcx
    testb       $0x8, %sil
    je          4f
    xor         (%rdi), %rax
    add         $0x8, %rdi
4:  testb       $0x4, %sil
    je          4f
    mov         (%rdi), %edx
    add         $0x4, %rdi
4:  testb       $0x2, %sil
    je          4f
    movzwl      (%rdi), %r10d
    shl         $0x10, %rdx
    add         $0x2, %rdi
    add         %r10, %rdx
4:  testb       $0x1, %sil
    je          4f
    movzbl      (%rdi), %r10d
    shl         $0x8, %rdx
    add         %r10, %rdx
4:  xor         %rdx, %rcx
    jmp 3b

    # Unaligned main-loop, process 32-byte unaligned chunks at a time
    # using 16-byte signed multiplication and unpack to mix.

.align 16
5:  movdqu      (%rdi), %xmm3
    movdqu      0x10(%rdi), %xmm4
    paddq       %xmm3, %xmm0
    paddq       %xmm4, %xmm1
    pmullw      %xmm2, %xmm0
    pmullw      %xmm2, %xmm1
    add         $0x20, %rdi
    movdqa      %xmm0, %xmm3
    punpckhbw   %xmm1, %xmm0
    punpcklbw   %xmm3, %xmm1
    dec         %rax
    jne         5b
    pxor        %xmm1, %xmm0
    jmp         2b

.size memhash, .-memhash
