/*===-- IPO.h - Interprocedural Transformations C Interface -----*- C++ -*-===*\
|*                                                                            *|
|*                     The LLVM Compiler Infrastructure                       *|
|*                                                                            *|
|* This file is distributed under the University of Illinois Open Source      *|
|* License. See LICENSE.TXT for details.                                      *|
|*                                                                            *|
|*===----------------------------------------------------------------------===*|
|*                                                                            *|
|* This header declares the C interface to libLLVMIPO.a, which implements     *|
|* various interprocedural transformations of the LLVM IR.                    *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifndef LLVM_C_TRANSFORMS_IPO_H
#define LLVM_C_TRANSFORMS_IPO_H

#include "llvm-c/Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup LLVMCTransformsIPO Interprocedural transformations
 * @ingroup LLVMCTransforms
 *
 * @{
 */

/** See llvm::createArgumentPromotionPass function. */
void LLVMAddArgumentPromotionPass(LLVMPassManagerRef PM);

/** See llvm::createConstantMergePass function. */
void LLVMAddConstantMergePass(LLVMPassManagerRef PM);

/** See llvm::createCalledValuePropagationPass function. */
void LLVMAddCalledValuePropagationPass(LLVMPassManagerRef PM);

/** See llvm::createDeadArgEliminationPass function. */
void LLVMAddDeadArgEliminationPass(LLVMPassManagerRef PM);

/** See llvm::createFunctionAttrsPass function. */
void LLVMAddFunctionAttrsPass(LLVMPassManagerRef PM);

/** See llvm::createFunctionInliningPass function. */
void LLVMAddFunctionInliningPass(LLVMPassManagerRef PM);

/** See llvm::createAlwaysInlinerPass function. */
void LLVMAddAlwaysInlinerPass(LLVMPassManagerRef PM);

/** See llvm::createGlobalDCEPass function. */
void LLVMAddGlobalDCEPass(LLVMPassManagerRef PM);

/** See llvm::createGlobalOptimizerPass function. */
void LLVMAddGlobalOptimizerPass(LLVMPassManagerRef PM);

/** See llvm::createIPConstantPropagationPass function. */
void LLVMAddIPConstantPropagationPass(LLVMPassManagerRef PM);

/** See llvm::createPruneEHPass function. */
void LLVMAddPruneEHPass(LLVMPassManagerRef PM);

/** See llvm::createIPSCCPPass function. */
void LLVMAddIPSCCPPass(LLVMPassManagerRef PM);

/** See llvm::createInternalizePass function. */
void LLVMAddInternalizePass(LLVMPassManagerRef, unsigned AllButMain);

/**
 * Create and add the internalize pass to the given pass manager with the
 * provided preservation callback.
 *
 * The context parameter is forwarded to the callback on each invocation.
 * As such, it is the responsibility of the caller to extend its lifetime
 * until execution of this pass has finished.
 *
 * @see llvm::createInternalizePass function.
 */
void LLVMAddInternalizePassWithMustPreservePredicate(
    LLVMPassManagerRef PM,
    void *Context,
    LLVMBool (*MustPreserve)(LLVMValueRef, void *));

/** See llvm::createStripDeadPrototypesPass function. */
void LLVMAddStripDeadPrototypesPass(LLVMPassManagerRef PM);

/** See llvm::createStripSymbolsPass function. */
void LLVMAddStripSymbolsPass(LLVMPassManagerRef PM);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* defined(__cplusplus) */

#endif
