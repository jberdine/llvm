/*===-- analysis_ocaml.c - LLVM OCaml Glue ----------------------*- C++ -*-===*\
|*                                                                            *|
|*                     The LLVM Compiler Infrastructure                       *|
|*                                                                            *|
|* This file is distributed under the University of Illinois Open Source      *|
|* License. See LICENSE.TXT for details.                                      *|
|*                                                                            *|
|*===----------------------------------------------------------------------===*|
|*                                                                            *|
|* This file glues LLVM's OCaml interface to its C interface. These functions *|
|* are by and large transparent wrappers to the corresponding C functions.    *|
|*                                                                            *|
|* Note that these functions intentionally take liberties with the CAMLparamX *|
|* macros, since most of the parameters are not GC heap objects.              *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#include "llvm-c/Analysis.h"
#include "llvm-c/Core.h"
#include "caml/alloc.h"
#include "caml/mlvalues.h"
#include "caml/memory.h"

/* Llvm.llmodule -> string option */
CAMLprim value llvm_verify_module(LLVMModuleRef M) {
  CAMLparam0();
  CAMLlocal2(String, Option);

  char *Message;
  int Result = LLVMVerifyModule(M, LLVMReturnStatusAction, &Message);

  if (0 == Result) {
    Option = Val_int(0);
  } else {
    Option = alloc(1, 0);
    String = copy_string(Message);
    Store_field(Option, 0, String);
  }

  LLVMDisposeMessage(Message);

  CAMLreturn(Option);
}

/* Llvm.llvalue -> bool */
CAMLprim value llvm_verify_function(LLVMValueRef Fn) {
  return Val_bool(LLVMVerifyFunction(Fn, LLVMReturnStatusAction) == 0);
}

/* Llvm.llmodule -> unit */
CAMLprim value llvm_assert_valid_module(LLVMModuleRef M) {
  LLVMVerifyModule(M, LLVMAbortProcessAction, 0);
  return Val_unit;
}

/* Llvm.llvalue -> unit */
CAMLprim value llvm_assert_valid_function(LLVMValueRef Fn) {
  LLVMVerifyFunction(Fn, LLVMAbortProcessAction);
  return Val_unit;
}

/* Llvm.llvalue -> unit */
CAMLprim value llvm_view_function_cfg(LLVMValueRef Fn) {
  LLVMViewFunctionCFG(Fn);
  return Val_unit;
}

/* Llvm.llvalue -> unit */
CAMLprim value llvm_view_function_cfg_only(LLVMValueRef Fn) {
  LLVMViewFunctionCFGOnly(Fn);
  return Val_unit;
}

/* Llvm.llcallgraph -> unit */
CAMLprim value llvm_dispose_callgraph(LLVMCallGraphCtx Cg) {
  disposeCallGraph(Cg);
  return Val_unit;
}

/* Llvm.llmodule -> Llvm.llcallgraph */
CAMLprim LLVMCallGraphCtx llvm_get_callgraph(LLVMModuleRef M) {
  return getCallGraph(M);
}

/* Llvm.llcallgraph -> Llvm.llmodule -> int */
CAMLprim value llvm_get_callee_size(LLVMCallGraphCtx CgCtx, LLVMValueRef Fn) {
  unsigned size = LLVMGetCalleeSize(CgCtx, Fn);
  return Val_int(size);
}

/* Llvm.llcallgraph -> Llvm.llmodule -> int */
CAMLprim value llvm_get_callee_num_refs(LLVMCallGraphCtx CgCtx, LLVMValueRef Fn) {
  unsigned numrefs= LLVMGetCalleeRefsNum(CgCtx, Fn);
  return Val_int(numrefs);
}

/* Llvm.llcallgraph -> Llvm.llvalue -> int -> Llvm.llvalue option */
CAMLprim value llvm_get_ith_callee(LLVMCallGraphCtx CgCtx, LLVMValueRef Fn, value Index) {
  CAMLparam0();
  CAMLlocal1(Option);
  LLVMValueRef ret = LLVMGetIthCallee(CgCtx, Fn, Int_val(Index));
  if(ret == NULL) {
    Option = Val_int(0);
  } else {
    Option = caml_alloc(1,0);
    Store_field(Option, 0, ret);
  }
  CAMLreturn(Option);
}

