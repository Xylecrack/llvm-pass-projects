#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct ArrayAccessPass : public PassInfoMixin<ArrayAccessPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    errs() << "Running ArrayAccessPass on function: " << F.getName() << "\n";

    auto &LI = FAM.getResult<LoopAnalysis>(F);
    auto &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);

    for (Loop *L : LI) {
      errs() << "Loop detected.\n";
      for (auto *BB : L->getBlocks()) {
        for (auto &I : *BB) {
          if (auto *GEP = dyn_cast<GetElementPtrInst>(&I)) {
            errs() << "  Found GEP: " << *GEP << "\n";

            // Get the SCEV expression for the pointer operand
            const SCEV *S = SE.getSCEV(GEP);

            errs() << "    SCEV: " << *S << "\n";

            // Check if it is an AddRecExpr (i.e., a recurrence across the loop)
            if (auto *AR = dyn_cast<SCEVAddRecExpr>(S)) {
              if (AR->getLoop() == L) {
                errs() << "    GEP varies across loop: yes\n";
                errs() << "    Step recurrence: " << *AR->getStepRecurrence(SE) << "\n";

                if (auto *ConstStep = dyn_cast<SCEVConstant>(AR->getStepRecurrence(SE))) {
                  auto stride = ConstStep->getValue()->getSExtValue();
                  if (stride == 1) {
                    errs() << "    Access is continuous (stride 1).\n";
                  } else {
                    errs() << "    Access has stride " << stride << " — not continuous.\n";
                  }
                } else {
                  errs() << "    Stride is not a constant — can't guarantee continuity.\n";
                }
              }
            } else {
              errs() << "    GEP is not a loop-varying recurrence — not continuous.\n";
            }
          }
        }
      }
    }

    return PreservedAnalyses::all();
  }
};

} // namespace

// Register with opt
extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "ArrayAccessPass", LLVM_VERSION_STRING,
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager &FPM,
           ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "array-access") {
            FPM.addPass(ArrayAccessPass());
            return true;
          }
          return false;
        });

      PB.registerAnalysisRegistrationCallback(
        [](FunctionAnalysisManager &FAM) {
          FAM.registerPass([] { return LoopAnalysis(); });
          FAM.registerPass([] { return ScalarEvolutionAnalysis(); });
        });
    }
  };
}
  