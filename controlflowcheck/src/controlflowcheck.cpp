
//===----------------------------------------------------------------------===//
//
// This file implements the Control Flow Checking by Software Signatures (CFCSS)
// instrumentation pass. It inserts XOR-based software signatures into the
// LLVM IR to detect control flow errors at runtime. The pass assigns unique
// signatures to basic blocks, maintains a global signature variable, and
// validates control flow integrity across blocks.
// REF: https://www.researchgate.net/publication/3152520_Control-flow_checking_by_software_signatures
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

namespace {

/// Global identifiers
// GName stores controlflow signatures at runtime.
static const char *GName = "__cfcss_G";
// HandlerName is called when cfcss fault is detected.
static const char *HandlerName = "__cfcss_fault";

/// Helper to get or insert global signature variable and fault handler.
std::pair<GlobalVariable *, Function *> getOrInsertRuntime(Module &M) {
  LLVMContext &Ctx = M.getContext();
  Type *Int32Ty = Type::getInt32Ty(Ctx);

  GlobalVariable *G = M.getNamedGlobal(GName);
  if (!G) {
    // Creates a global variable(GName) which is mutable and of type i32.
    G = new GlobalVariable(M, Int32Ty, false, GlobalValue::ExternalLinkage,
                           ConstantInt::get(Int32Ty, 0), GName);
  }
  // Declares/retrives cfcss runtime handler (runtime/cfcss_rt.c).
  FunctionCallee Callee = M.getOrInsertFunction(
      HandlerName, FunctionType::get(Type::getVoidTy(Ctx), false));
  return {G, cast<Function>(Callee.getCallee())};
}

struct ControlFlowCheckPass : PassInfoMixin<ControlFlowCheckPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    // Ignores external functions like printf() etc.
    if (F.isDeclaration())
      return PreservedAnalyses::all();

   // split crictical edges
    SmallVector<std::pair<Instruction *, unsigned>, 4> CriticalEdges;

    for (BasicBlock &BB : F) {
      Instruction *TI = BB.getTerminator();
      // source must have multiple successors
      if (TI->getNumSuccessors() > 1) {
        for (unsigned i = 0; i < TI->getNumSuccessors(); ++i) {
          BasicBlock *Dest = TI->getSuccessor(i);
          // destination must have multiple predecessors
          // getSinglePredecessor returns nullptr if there are > 1 predecessors.
          if (Dest->getSinglePredecessor() == nullptr) {
            CriticalEdges.push_back({TI, i});
          }
        }
      }
    }

    // Apply the splits
    // We use default CriticalEdgeSplittingOptions() which passes nullptrs.
    // This avoids the type mismatch error.
    for (auto &Edge : CriticalEdges) {
      SplitCriticalEdge(Edge.first, Edge.second,
                        CriticalEdgeSplittingOptions());
    }

    // Refresh references after CFG modification
    Module &M = *F.getParent();
    auto [GVar, Handler] = getOrInsertRuntime(M);
    LLVMContext &Ctx = M.getContext();
    Type *Int32Ty = Type::getInt32Ty(Ctx);

   // snapshot the cfg preds
    DenseMap<BasicBlock *, SmallVector<BasicBlock *, 4>> OrigPreds;
    for (BasicBlock &BB : F) {
      for (BasicBlock *Pred : predecessors(&BB)) {
        OrigPreds[&BB].push_back(Pred);
      }
    }

    // Step 1: Assign unique signature to each basic block.
    DenseMap<BasicBlock *, uint32_t> blockSign;
    uint32_t sig = 0;
    for (BasicBlock &BB : F) {
      blockSign[&BB] = ++sig;
    }

    // Step 2: Create common error block
    BasicBlock *ErrorBB = BasicBlock::Create(Ctx, "cfcss.error", &F);
    IRBuilder<> EB(ErrorBB);
    EB.CreateCall(Handler);
    EB.CreateUnreachable();

    // Step 3 : Instrumentation pass over original basic blocks
    // Iterating over basic blocks which are created on runtime
    // creates a infinite loop of checking and adding new BB
    // hence, OriginalBBs is used to keep track of original Basic blocks
    SmallVector<BasicBlock *, 16> WorkList;
    for (BasicBlock &BB : F)
      if (&BB != ErrorBB)
        WorkList.push_back(&BB);

    for (BasicBlock *BB : WorkList) {
      auto It = BB->getFirstNonPHIOrDbgOrLifetime();
      if (It == BB->end())
        continue;

      IRBuilder<> Builder(BB, It);
      Value *Gval = nullptr;

      auto &Preds = OrigPreds[BB];
      // Step 3.1: Choose base predecessor
      if (Preds.empty()) {
        uint32_t s = blockSign[BB];
        Builder.CreateStore(ConstantInt::get(Int32Ty, s), GVar);
        Gval = ConstantInt::get(Int32Ty, s);
      } else {
        BasicBlock *BasePred = Preds[0];
        // Step 3.2: Insert D-setting in non-base preds
        uint32_t sPred = blockSign.count(BasePred) ? blockSign[BasePred] : 0;
        uint32_t sCurr = blockSign[BB];
        uint32_t d = sPred ^ sCurr;

        if (Preds.size() > 1) {
          for (BasicBlock *P : Preds) {
            if (P == BasePred)
              continue;

            uint32_t sP = blockSign.count(P) ? blockSign[P] : 0;
            uint32_t D = sPred ^ sP;

            Instruction *T = P->getTerminator();
            IRBuilder<> PB(T);
            Value *OldG = PB.CreateLoad(Int32Ty, GVar);
            Value *AdjG = PB.CreateXor(OldG, ConstantInt::get(Int32Ty, D));
            PB.CreateStore(AdjG, GVar);
          }
        }

        Gval = Builder.CreateLoad(Int32Ty, GVar);
        if (d != 0) {
          Gval = Builder.CreateXor(Gval, ConstantInt::get(Int32Ty, d));
          Builder.CreateStore(Gval, GVar);
        }
      }

      // Step 3.4: Compare signature and branch to error if mismatch
      if (!Preds.empty()) {
        Value *Check = Builder.CreateICmpNE(
            Gval, ConstantInt::get(Int32Ty, blockSign[BB]));

        SplitBlockAndInsertIfThen(Check, &*It, false, nullptr, nullptr, nullptr,
                                  ErrorBB);
      }
    }

    return PreservedAnalyses::none();
  }
};

} // namespace

/// Register the pass with the new pass manager
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "CFCSS", "0.1", [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "cfcss") {
                    FPM.addPass(ControlFlowCheckPass());
                    return true;
                  }
                  return false;
                });
          }};
}
