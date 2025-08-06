#ifndef ARRAYACCESS_H
#define ARRAYACCESS_H

#include "llvm/IR/PassManager.h"

struct ArrayAccessPass : public llvm::PassInfoMixin<::ArrayAccessPass> {
  llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM);
};

#endif // ARRAYACCESS_H
