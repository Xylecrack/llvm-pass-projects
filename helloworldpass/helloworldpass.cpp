#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
// PassInfoMixin is a helper class template.
// gives name() feature to pass.
// makes the pass available for debugger or err stream to access and print the
// pass name via printPipeline()
struct HelloWorldPass : public PassInfoMixin<HelloWorldPass> {
  // PreservedAnalyses is return type of run()
  // tells the llvm backend if the ir has been modified by this pass
  // if modified, analysis like scalaerevolution,loopinfo etc may need to be
  // recomputed.
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    size_t numBBs = F.size();
    size_t numIs = 0;
    for (auto &BB : F) {
      numIs += BB.size();
    }
    errs() << "Function:" << F.getName() << "\n";
    errs() << "  Basic Blocks: " << numBBs << "\n";
    errs() << "  Instructions: " << numIs << "\n";
    // IR has not beed modified
    // all existing analysis still holds
    return PreservedAnalyses::all();
  }
};

/// Register the pass with the new pass manager
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "HelloWorldPass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "hello-world") {
                    FPM.addPass(HelloWorldPass());
                    return true;
                  }
                  return false;
                });
          }};
}