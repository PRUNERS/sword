//===------ RegisterPasses.cpp - Add the Sword Passes to default passes  --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file composes the individual LLVM-IR passes provided by Sword to a
// functional polyhedral optimizer. The polyhedral optimizer is automatically
// made available to LLVM based compilers by loading the Sword shared library
// into such a compiler.
//
// The Sword optimizer is made available by executing a static constructor that
// registers the individual Sword passes in the LLVM pass manager builder. The
// passes are registered such that the default behaviour of the compiler is not
// changed, but that the flag '-polly' provided at optimization level '-O3'
// enables additional polyhedral optimizations.
//===----------------------------------------------------------------------===//

#include "../../include/sword/LinkAllPasses.h"
#include "llvm/Analysis/CFGPrinter.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Vectorize.h"

namespace llvm {
void initializeSwordPasses(llvm::PassRegistry &Registry) {
  initializeInstrumentParallelPass(Registry);
}

void registerSwordPasses(llvm::legacy::PassManagerBase &PM) {
  PM.add(createInstrumentParallelPass());
}
}
