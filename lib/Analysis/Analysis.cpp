//===-- Analysis.cpp ------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm-c/Analysis.h"
#include "llvm-c/Initialization.h"
#include "llvm/IR/Module.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Verifier.h"
#include "llvm/InitializePasses.h"
#include "llvm/PassRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <cstring>

using namespace llvm;

/// initializeAnalysis - Initialize all passes linked into the Analysis library.
void llvm::initializeAnalysis(PassRegistry &Registry) {
  initializeAAEvalLegacyPassPass(Registry);
  initializeAliasSetPrinterPass(Registry);
  initializeBasicAAWrapperPassPass(Registry);
  initializeBlockFrequencyInfoWrapperPassPass(Registry);
  initializeBranchProbabilityInfoWrapperPassPass(Registry);
  initializeCallGraphWrapperPassPass(Registry);
  initializeCallGraphDOTPrinterPass(Registry);
  initializeCallGraphPrinterLegacyPassPass(Registry);
  initializeCallGraphViewerPass(Registry);
  initializeCostModelAnalysisPass(Registry);
  initializeCFGViewerLegacyPassPass(Registry);
  initializeCFGPrinterLegacyPassPass(Registry);
  initializeCFGOnlyViewerLegacyPassPass(Registry);
  initializeCFGOnlyPrinterLegacyPassPass(Registry);
  initializeCFLAndersAAWrapperPassPass(Registry);
  initializeCFLSteensAAWrapperPassPass(Registry);
  initializeDependenceAnalysisWrapperPassPass(Registry);
  initializeDelinearizationPass(Registry);
  initializeDemandedBitsWrapperPassPass(Registry);
  initializeDominanceFrontierWrapperPassPass(Registry);
  initializeDomViewerPass(Registry);
  initializeDomPrinterPass(Registry);
  initializeDomOnlyViewerPass(Registry);
  initializePostDomViewerPass(Registry);
  initializeDomOnlyPrinterPass(Registry);
  initializePostDomPrinterPass(Registry);
  initializePostDomOnlyViewerPass(Registry);
  initializePostDomOnlyPrinterPass(Registry);
  initializeAAResultsWrapperPassPass(Registry);
  initializeGlobalsAAWrapperPassPass(Registry);
  initializeIVUsersWrapperPassPass(Registry);
  initializeInstCountPass(Registry);
  initializeIntervalPartitionPass(Registry);
  initializeLazyBranchProbabilityInfoPassPass(Registry);
  initializeLazyBlockFrequencyInfoPassPass(Registry);
  initializeLazyValueInfoWrapperPassPass(Registry);
  initializeLazyValueInfoPrinterPass(Registry);
  initializeLegacyDivergenceAnalysisPass(Registry);
  initializeLintPass(Registry);
  initializeLoopInfoWrapperPassPass(Registry);
  initializeMemDepPrinterPass(Registry);
  initializeMemDerefPrinterPass(Registry);
  initializeMemoryDependenceWrapperPassPass(Registry);
  initializeModuleDebugInfoPrinterPass(Registry);
  initializeModuleSummaryIndexWrapperPassPass(Registry);
  initializeMustExecutePrinterPass(Registry);
  initializeObjCARCAAWrapperPassPass(Registry);
  initializeOptimizationRemarkEmitterWrapperPassPass(Registry);
  initializePhiValuesWrapperPassPass(Registry);
  initializePostDominatorTreeWrapperPassPass(Registry);
  initializeRegionInfoPassPass(Registry);
  initializeRegionViewerPass(Registry);
  initializeRegionPrinterPass(Registry);
  initializeRegionOnlyViewerPass(Registry);
  initializeRegionOnlyPrinterPass(Registry);
  initializeSCEVAAWrapperPassPass(Registry);
  initializeScalarEvolutionWrapperPassPass(Registry);
  initializeTargetTransformInfoWrapperPassPass(Registry);
  initializeTypeBasedAAWrapperPassPass(Registry);
  initializeScopedNoAliasAAWrapperPassPass(Registry);
  initializeLCSSAVerificationPassPass(Registry);
  initializeMemorySSAWrapperPassPass(Registry);
  initializeMemorySSAPrinterLegacyPassPass(Registry);
}

void LLVMInitializeAnalysis(LLVMPassRegistryRef R) {
  initializeAnalysis(*unwrap(R));
}

void LLVMInitializeIPA(LLVMPassRegistryRef R) {
  initializeAnalysis(*unwrap(R));
}

LLVMBool LLVMVerifyModule(LLVMModuleRef M, LLVMVerifierFailureAction Action,
                          char **OutMessages) {
  raw_ostream *DebugOS = Action != LLVMReturnStatusAction ? &errs() : nullptr;
  std::string Messages;
  raw_string_ostream MsgsOS(Messages);

  LLVMBool Result = verifyModule(*unwrap(M), OutMessages ? &MsgsOS : DebugOS);

  // Duplicate the output to stderr.
  if (DebugOS && OutMessages)
    *DebugOS << MsgsOS.str();

  if (Action == LLVMAbortProcessAction && Result)
    report_fatal_error("Broken module found, compilation aborted!");

  if (OutMessages)
    *OutMessages = strdup(MsgsOS.str().c_str());

  return Result;
}

LLVMBool LLVMVerifyFunction(LLVMValueRef Fn, LLVMVerifierFailureAction Action) {
  LLVMBool Result = verifyFunction(
      *unwrap<Function>(Fn), Action != LLVMReturnStatusAction ? &errs()
                                                              : nullptr);

  if (Action == LLVMAbortProcessAction && Result)
    report_fatal_error("Broken function found, compilation aborted!");

  return Result;
}

void LLVMViewFunctionCFG(LLVMValueRef Fn) {
  Function *F = unwrap<Function>(Fn);
  F->viewCFG();
}

void LLVMViewFunctionCFGOnly(LLVMValueRef Fn) {
  Function *F = unwrap<Function>(Fn);
  F->viewCFGOnly();
}
DEFINE_SIMPLE_CONVERSION_FUNCTIONS(CallGraph, LLVMCallGraphCtx)

LLVMCallGraphCtx getCallGraph(LLVMModuleRef M) {
  auto cg = new CallGraph(*unwrap(M));
  return wrap(cg);
}

void disposeCallGraph(LLVMCallGraphCtx cg) {
  delete unwrap(cg);
}

unsigned LLVMGetCalleeSize(LLVMCallGraphCtx cgctx, LLVMValueRef Fn) {
  CallGraph* cg = unwrap(cgctx);
  if (const Function *F = dyn_cast<Function>(unwrap(Fn))) {
    return (*cg)[F]->size();

  }
  assert(false && "Passed a non function to get calles");
  return 0;
}

unsigned LLVMGetCalleeRefsNum(LLVMCallGraphCtx cgctx, LLVMValueRef Fn) {
  CallGraph* cg = unwrap(cgctx);
  if (const Function *F = dyn_cast<Function>(unwrap(Fn))) {
    return (*cg)[F]->getNumReferences();

  }
  assert(false && "Passed a non function to get calles");
  return 0;
}
LLVMValueRef LLVMGetIthCallee(LLVMCallGraphCtx cgctx, LLVMValueRef Fn, unsigned index) {
  CallGraph* cg = unwrap(cgctx);
  if (const Function *F = dyn_cast<Function>(unwrap(Fn))) {
    assert(index < (*cg)[F]->size() && "Callee index out of bounds");
    CallGraphNode *cgn = (*cg)[F];
    CallGraphNode *calledCgn = (*cgn)[index];
    return wrap(calledCgn->getFunction());

  }
  assert(false && "Passed a non function to get calles");
  return nullptr;
}
