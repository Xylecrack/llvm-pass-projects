
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
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

namespace {
// Dump the predecessors and successors of all the basic blocks in an IR
/* void infoFunction(Function &F) {
  errs() << "[CFG INFO] Function: " << F.getName() << "\n";

  for (BasicBlock &BB : F) {
    errs() << "  [CFG] " << BB.getName() << " successors: ";
    for (succ_iterator SI = succ_begin(&BB), SE = succ_end(&BB); SI != SE;
         ++SI) {
      errs() << (*SI)->getName() << " ";
    }
    errs() << "\n";
    errs() << "        " << BB.getName() << " predecessors: ";
    for (pred_iterator PI = pred_begin(&BB), PE = pred_end(&BB); PI != PE;
         ++PI) {
      errs() << (*PI)->getName() << " ";
    }
    errs() << "\n";
  }
  errs() << "[CFG INFO END]\n";
} */

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

    Module &M = *F.getParent();
    auto [GVar, Handler] = getOrInsertRuntime(M);
    LLVMContext &Ctx = M.getContext();
    Type *Int32Ty = Type::getInt32Ty(Ctx);

    // Step 1: Assign unique signature to each basic block.
    DenseMap<BasicBlock *, uint32_t> blockSign;
    uint32_t sig = 0;
    for (BasicBlock &BB : F) {
      blockSign[&BB] = sig++;
    }

    // Step 2: Create common error block
    BasicBlock *ErrorBB = BasicBlock::Create(Ctx, "cfcss.error", &F);
    IRBuilder<> EB(ErrorBB);
    EB.CreateCall(Handler);
    EB.CreateUnreachable();

    // Step 3 : Instrumentation pass over original basic blocks
    // Iterating over basic blocks which are created on runtime
    // creats a infinite loop of checking and adding new BB
    // hence, OriginalBBs is used to keep track of original Basic blocks
    SmallVector<BasicBlock *, 16> OriginalBBs;
    for (BasicBlock &BB : F) {
      OriginalBBs.push_back(&BB);
    }

    for (BasicBlock *BB : OriginalBBs) {
      SmallVector<BasicBlock *, 8> preds(pred_begin(BB), pred_end(BB));

      // Step 3.1: Choose base predecessor
      uint32_t d = 0;
      BasicBlock *basePred = nullptr;
      if (!preds.empty()) {
        basePred = preds[0];
        d = blockSign[basePred] ^ blockSign[BB];
      }

      // Step 3.2: Insert D-setting in non-base preds
      if (preds.size() > 1) {
        uint32_t baseSign = blockSign[basePred];
        for (BasicBlock *pred : preds) {
          if (pred == basePred)
            continue;
          uint32_t D = baseSign ^ blockSign[pred];

          IRBuilder<> PredBuilder(pred->getTerminator());
          Value *Gload = PredBuilder.CreateLoad(Int32Ty, GVar, "Gload_D");
          Value *NewG = PredBuilder.CreateXor(
              Gload, ConstantInt::get(Int32Ty, D), "xor_D");
          PredBuilder.CreateStore(NewG, GVar);
        }
      }

      // Step 3.3: Insert XOR d and check at BB start
      Instruction *InsertPt = BB->getFirstNonPHIOrDbgOrLifetime();
      if (!InsertPt) {
        errs() << "[CFCSS] Warning: No valid insert point in BB: "
               << BB->getName() << "\n";
        continue;
      }

      IRBuilder<> Builder(BB, InsertPt->getIterator());
      Value *Gload = nullptr;

      if (d != 0) {
        Gload = Builder.CreateLoad(Int32Ty, GVar, "Gload");
        Value *NewG =
            Builder.CreateXor(Gload, ConstantInt::get(Int32Ty, d), "xor_d");
        Builder.CreateStore(NewG, GVar);
        Gload = NewG;
      } else {
        Gload = Builder.CreateLoad(Int32Ty, GVar, "Gload");
      }

      // Step 3.4: Compare signature and branch to error if mismatch
      Value *Cmp = Builder.CreateICmpNE(
          Gload, ConstantInt::get(Int32Ty, blockSign[BB]), "cmp_sig");
      SplitBlockAndInsertIfThen(Cmp, Builder.GetInsertPoint(), false, nullptr,
                                nullptr, nullptr, ErrorBB);
    }
    /*  infoFunction(F); */
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
