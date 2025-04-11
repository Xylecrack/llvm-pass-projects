#include "arrayaccess.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

PreservedAnalyses ArrayAccessPass::run(Function &F, FunctionAnalysisManager &FAM)
{
    errs() << "Running ArrayAccessPass on function: " << F.getName() << "\n";

    auto &LI = FAM.getResult<LoopAnalysis>(F);
    auto &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
    const DataLayout &DL = F.getParent()->getDataLayout();
    for (auto &B : F)
    {
        for (auto &I : B)
        {
            ArrayAccessOffset(&I, &SE);
        }
    }
    for (Loop *L : LI)
    {
        errs() << "Loop detected.\n";
        for (auto *BB : L->getBlocks())
        {
            for (auto &I : *BB)
            {

                if (auto *GEP = dyn_cast<GetElementPtrInst>(&I))
                {
                    errs() << "  Found GEP: " << *GEP << "\n";

                    const SCEV *S = SE.getSCEV(GEP);
                    errs() << "    SCEV: " << *S << "\n";

                    if (auto *AR = dyn_cast<SCEVAddRecExpr>(S))
                    {
                        if (AR->getLoop() == L)
                        {
                            errs() << "    GEP varies across loop: yes\n";
                            errs() << "    Step recurrence: " << *AR->getStepRecurrence(SE) << "\n";

                            if (auto *ConstStep = dyn_cast<SCEVConstant>(AR->getStepRecurrence(SE)))
                            {
                                auto stride = ConstStep->getValue()->getSExtValue();
                                Type *ElementType = GEP->getSourceElementType();
                                uint64_t ElementSize = DL.getTypeAllocSize(ElementType);

                                errs() << "    Raw stride (bytes): " << stride << "\n";
                                errs() << "    Element size: " << ElementSize << " bytes\n";

                                if (ElementSize != 0 && stride % ElementSize == 0)
                                {
                                    auto logicalStride = stride / ElementSize;
                                    errs() << "    Logical stride (elements): " << logicalStride << "\n";

                                    if (logicalStride == 1)
                                    {
                                        errs() << "    Access is logically continuous (1 element per iteration).\n";
                                    }
                                    else
                                    {
                                        errs() << "    Access is strided — " << logicalStride
                                               << " elements per iteration.\n";
                                    }
                                }
                                else
                                {
                                    errs() << "    Stride is not a multiple of element size — can't determine logical "
                                              "continuity.\n";
                                }
                            }
                            else
                            {
                                errs() << "    Stride is not a constant — can't guarantee continuity.\n";
                            }
                        }
                    }
                    else
                    {
                        errs() << "    GEP is not a loop-varying recurrence — not continuous.\n";
                    }
                }
            }
        }
    }

    return PreservedAnalyses::all();
}
bool ArrayAccessPass::ArrayAccessOffset(Instruction *I, ScalarEvolution *SE)
{
    if (I->getOpcode() != Instruction::Add)
        return false;

    const SCEV *S = SE->getSCEV(I);
    if (const SCEVAddRecExpr *SARE = dyn_cast<SCEVAddRecExpr>(S))
    {
        if (SARE->isAffine())
        {
            errs() << "Affine SCEV: " << *SARE << "\n";
        }
    }

    return false;
}
// Register with opt
extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo()
{
    return {LLVM_PLUGIN_API_VERSION, "ArrayAccessPass", LLVM_VERSION_STRING, [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
                        if (Name == "array-access")
                        {
                            FPM.addPass(ArrayAccessPass());
                            return true;
                        }
                        return false;
                    });

                PB.registerAnalysisRegistrationCallback([](FunctionAnalysisManager &FAM) {
                    FAM.registerPass([] { return LoopAnalysis(); });
                    FAM.registerPass([] { return ScalarEvolutionAnalysis(); });
                });
            }};
}
