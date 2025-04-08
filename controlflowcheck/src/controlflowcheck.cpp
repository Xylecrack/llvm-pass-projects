#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

using namespace llvm;

namespace
{

struct ControlFlowCheckPass : public PassInfoMixin<ControlFlowCheckPass>
{
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM)
    {
        errs() << "Running Control Flow Checking Pass on function: " << F.getName() << "\n";

        DenseMap<BasicBlock *, int> blockSign;
        DenseMap<BasicBlock *, int> signDiff;
        DenseMap<std::pair<BasicBlock *, BasicBlock *>, int> runtimeAdjustSign;

        int sign = 1;

        // Step 1: Assign unique signatures
        for (BasicBlock &BB : F)
            blockSign[&BB] = ++sign;

        // Step 2: Compute signature differences
        for (BasicBlock &BB : F)
        {
            auto preds = predecessors(&BB);
            if (!preds.empty())
            {
                BasicBlock *vi1 = *preds.begin();
                signDiff[&BB] = blockSign[vi1] ^ blockSign[&BB];
                for (BasicBlock *vim : preds)
                {
                    if (vim != vi1)
                    {
                        runtimeAdjustSign[{vim, &BB}] = blockSign[vi1] ^ blockSign[vim];
                    }
                }
            }
        }

        LLVMContext &Context = F.getContext();
        IntegerType *Int32Ty = Type::getInt32Ty(Context);

        // Step 3: Create error block
        BasicBlock *errorBlock = BasicBlock::Create(Context, "errorBlock", &F);
        IRBuilder<> errBuilder(errorBlock);
        Function *trapFn = Intrinsic::getDeclaration(F.getParent(), Intrinsic::trap);
        errBuilder.CreateCall(trapFn);
        errBuilder.CreateUnreachable();

        // Step 4: Create and initialize runtime signature
        IRBuilder<> entryBuilder(&F.getEntryBlock(), F.getEntryBlock().begin());
        AllocaInst *runtimeSign = entryBuilder.CreateAlloca(Int32Ty, nullptr, "__cfc_runtime_sign");
        entryBuilder.CreateStore(ConstantInt::get(Int32Ty, 0), runtimeSign);

        // Step 5: Instrument basic blocks
        for (BasicBlock &BB : F)
        {
            IRBuilder<> builder(&BB, BB.getFirstInsertionPt());

            Value *currSign = builder.CreateLoad(Int32Ty, runtimeSign);
            Value *blockSig = builder.getInt32(blockSign[&BB]);
            Value *updatedSign = builder.CreateXor(currSign, blockSig);
            builder.CreateStore(updatedSign, runtimeSign);

            Instruction *term = BB.getTerminator();
            if (!term) continue;

            SmallVector<BasicBlock *, 4> Succs(successors(&BB));

            if (Succs.size() == 1)
            {
                BasicBlock *Succ = Succs[0];
                Value *expected = builder.CreateXor(updatedSign, builder.getInt32(signDiff[Succ]));
                Value *actual = builder.CreateLoad(Int32Ty, runtimeSign);
                Value *cmp = builder.CreateICmpNE(actual, expected);
                IRBuilder<> termBuilder(term);
                termBuilder.CreateCondBr(cmp, errorBlock, Succ);
                term->eraseFromParent();
            }
            else if (Succs.size() > 1)
            {
                BranchInst *branch = dyn_cast<BranchInst>(term);
                if (!branch || !branch->isConditional()) continue;

                Value *cond = branch->getCondition();
                BasicBlock *trueDest = branch->getSuccessor(0);
                BasicBlock *falseDest = branch->getSuccessor(1);

                IRBuilder<> termBuilder(term);
                for (BasicBlock *Succ : {trueDest, falseDest})
                {
                    Value *expected = builder.CreateXor(updatedSign, builder.getInt32(signDiff[Succ]));
                    Value *actual = builder.CreateLoad(Int32Ty, runtimeSign);
                    Value *cmp = builder.CreateICmpNE(actual, expected);

                    auto it = runtimeAdjustSign.find({&BB, Succ});
                    if (it != runtimeAdjustSign.end())
                    {
                        Value *adj = builder.getInt32(it->second);
                        Value *newSign = builder.CreateXor(updatedSign, adj);
                        builder.CreateStore(newSign, runtimeSign);
                    }

                    // Replace terminator
                    Instruction *newBr = builder.CreateCondBr(cmp, errorBlock, Succ);
                    term->replaceAllUsesWith(newBr);
                    term->eraseFromParent();
                }
            }
        }

        return PreservedAnalyses::none();
    }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo()
{
    return {LLVM_PLUGIN_API_VERSION, "controlflowcheck", LLVM_VERSION_STRING, [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
                        if (Name == "control-flow-check")
                        {
                            FPM.addPass(ControlFlowCheckPass());
                            return true;
                        }
                        return false;
                    });
            }};
}
