#ifndef ARRAYACCESS_PASS_H
#define ARRAYACCESS_PASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Instructions.h"
namespace llvm {

struct ArrayAccessPass : public PassInfoMixin<ArrayAccessPass> {
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
    bool ArrayAccessOffset(Instruction *I, ScalarEvolution *SE);

};

} // namespace llvm

#endif // ARRAYACCESS_PASS_H