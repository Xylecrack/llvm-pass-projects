#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>

using namespace llvm;

namespace {

// Helper struct to hold analyzed information about a store instruction.
struct StoreInfo {
  StoreInst *Instruction;
  Value *BasePointer;
  int64_t ConstantOffset;

  // Equality operator for comparing vectors of StoreInfo.
  bool operator==(const StoreInfo &other) const {
    return Instruction == other.Instruction &&
           BasePointer == other.BasePointer &&
           ConstantOffset == other.ConstantOffset;
  }
};

// The main pass structure.
struct ArrayAccessPass : public PassInfoMixin<ArrayAccessPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};

PreservedAnalyses ArrayAccessPass::run(Function &F,
                                       FunctionAnalysisManager &FAM) {
  // Get the analysis results we need.
  auto &LI = FAM.getResult<LoopAnalysis>(F);
  auto &DI = FAM.getResult<DependenceAnalysis>(F);
  auto &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);

  // Process each top-level loop in the function.
  for (Loop *L : LI) {
    errs() << "--- Analyzing Loop with header: ";
    L->getHeader()->printAsOperand(errs(), false);
    errs() << " ---\n";

    // 1. Collect all memory instructions within the loop for dependency checks.
    SmallVector<Instruction *, 16> MemInstr;
    for (BasicBlock *BB : L->getBlocks()) {
      for (Instruction &I : *BB) {
        if (isa<LoadInst>(I) || isa<StoreInst>(I)) {
          MemInstr.push_back(&I);
        }
      }
    }

    // 2. Check for loop-independent dependencies that would make reordering unsafe.
    bool hasUnsafeDep = false;
    for (size_t i = 0; i < MemInstr.size(); ++i) {
      for (size_t j = i + 1; j < MemInstr.size(); ++j) {
        if (auto D = DI.depends(MemInstr[i], MemInstr[j], true)) {
          if (D->isOrdered() && D->isLoopIndependent()) {
            errs() << "Found LOOP-INDEPENDENT dependency. Cannot reorder safely.\n";
            hasUnsafeDep = true;
            break;
          }
        }
      }
      if (hasUnsafeDep) break;
    }
    if (hasUnsafeDep) continue;

    // 3. Use Scalar Evolution to analyze each store and group them by basic block.
    DenseMap<BasicBlock *, SmallVector<StoreInfo, 8>> BlockToStoresMap;
    for (Instruction *Inst : MemInstr) {
      if (StoreInst *SI = dyn_cast<StoreInst>(Inst)) {
        Value *Ptr = SI->getPointerOperand();
        const SCEV *S = SE.getSCEV(Ptr);

        // Find the ultimate base pointer of the address expression.
        const SCEV *BasePointerSCEV = SE.getPointerBase(S);
        Value *BasePtr = nullptr;
        if (const SCEVUnknown *BaseUnknown = dyn_cast<SCEVUnknown>(BasePointerSCEV)) {
          BasePtr = BaseUnknown->getValue();
        }
        if (!BasePtr) continue;

        // Calculate the offset from the base pointer.
        const SCEV *OffsetExpr = SE.getMinusSCEV(S, SE.getSCEV(BasePtr));

        // Handle both loop-variant (AddRec) and loop-invariant (Constant) offsets.
        if (const SCEVAddRecExpr *AddRec = dyn_cast<SCEVAddRecExpr>(OffsetExpr)) {
          if (const SCEVConstant *Const = dyn_cast<SCEVConstant>(AddRec->getStart())) {
            BlockToStoresMap[SI->getParent()].push_back({SI, BasePtr, Const->getAPInt().getSExtValue()});
          }
        } else if (const SCEVConstant *Const = dyn_cast<SCEVConstant>(OffsetExpr)) {
          BlockToStoresMap[SI->getParent()].push_back({SI, BasePtr, Const->getAPInt().getSExtValue()});
        }
      }
    }

    // 4. For each block with multiple stores, check order and reorder if necessary.
    for (auto &Pair : BlockToStoresMap) {
      BasicBlock *BB = Pair.getFirst();
      auto &StoreInfos = Pair.getSecond();
      if (StoreInfos.size() < 2) continue;

      errs() << "\nAnalyzing stores in block: ";
      BB->printAsOperand(errs(), false);
      errs() << "\n";

      // Sort by base pointer first, then by the constant offset.
      auto comparator = [](const StoreInfo &a, const StoreInfo &b) {
        if (a.BasePointer != b.BasePointer)
          return a.BasePointer < b.BasePointer;
        return a.ConstantOffset < b.ConstantOffset;
      };

      SmallVector<StoreInfo, 8> SortedStores = StoreInfos;
      std::sort(SortedStores.begin(), SortedStores.end(), comparator);

      if (SortedStores == StoreInfos) {
        errs() << " -> Access pattern in this block is already sorted.\n";
      } else {
        errs() << " -> Unsorted memory access detected. Rearranging instructions...\n";

        // Print the original order of instructions.
        errs() << "Before rearrangement:\n";
        for (const auto &info : StoreInfos) {
          info.Instruction->print(errs());
          errs() << "\n";
        }
        Instruction *InsertBefore = BB->getTerminator();
        for (const auto &info : SortedStores) {
          info.Instruction->removeFromParent();
          info.Instruction->insertBefore(InsertBefore);
        }

        // Print the new, sorted order of instructions.
        errs() << "After rearrangement:\n";
        for (const auto &info : SortedStores) {
          info.Instruction->print(errs());
          errs() << "\n";
        }
      }
    }
  }
  return PreservedAnalyses::none();
}

} // namespace

/// Register the pass with the new pass manager
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "ArrayAccessPass", LLVM_VERSION_STRING,
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
                  FAM.registerPass([] { return DependenceAnalysis(); });
                });
          }};
}