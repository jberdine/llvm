(*===-- llvm_analysis.ml - LLVM OCaml Interface ---------------*- OCaml -*-===*
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 *===----------------------------------------------------------------------===*)


external verify_module : Llvm.llmodule -> string option = "llvm_verify_module"

external verify_function : Llvm.llvalue -> bool = "llvm_verify_function"

external assert_valid_module : Llvm.llmodule -> unit
                             = "llvm_assert_valid_module"

external assert_valid_function : Llvm.llvalue -> unit
                               = "llvm_assert_valid_function"
external view_function_cfg : Llvm.llvalue -> unit = "llvm_view_function_cfg"
external view_function_cfg_only : Llvm.llvalue -> unit
                                = "llvm_view_function_cfg_only"

external get_callgraph : Llvm.llmodule -> Llvm.llcallgraph = "llvm_get_callgraph"
external dispose_callgraph : Llvm.llcallgraph -> unit = "llvm_dispose_callgraph"
external get_callee_size : Llvm.llcallgraph -> Llvm.llvalue -> int = "llvm_get_callee_size"
external get_callee_numrefs : Llvm.llcallgraph -> Llvm.llvalue -> int = "llvm_get_callee_num_refs"
external get_ith_callee : Llvm.llcallgraph -> Llvm.llvalue -> int -> Llvm.llvalue option = "llvm_get_ith_callee"

let fold_callees : Llvm.llcallgraph -> ('a -> Llvm.llvalue -> 'a) -> Llvm.llvalue -> 'a -> 'a =
  fun callgraph f func init ->
  let rec cg_fold_helper cg acc f func i =
    if i = 0 then acc
    else
      let ith_callee = get_ith_callee cg func (i - 1) in
      match ith_callee with
      | None -> cg_fold_helper callgraph acc f func (i - 1)
      | Some x -> cg_fold_helper callgraph (f acc x) f func (i - 1)
  in
  cg_fold_helper callgraph init f func (get_callee_size callgraph func)
