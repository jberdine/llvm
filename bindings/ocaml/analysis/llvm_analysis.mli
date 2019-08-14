(*===-- llvm_analysis.mli - LLVM OCaml Interface --------------*- OCaml -*-===*
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 *===----------------------------------------------------------------------===*)

(** Intermediate representation analysis.

    This interface provides an OCaml API for LLVM IR analyses, the classes in
    the Analysis library. *)

(** [verify_module m] returns [None] if the module [m] is valid, and
    [Some reason] if it is invalid. [reason] is a string containing a
    human-readable validation report. See [llvm::verifyModule]. *)
external verify_module : Llvm.llmodule -> string option = "llvm_verify_module"

(** [verify_function f] returns [None] if the function [f] is valid, and
    [Some reason] if it is invalid. [reason] is a string containing a
    human-readable validation report. See [llvm::verifyFunction]. *)
external verify_function : Llvm.llvalue -> bool = "llvm_verify_function"

(** [verify_module m] returns if the module [m] is valid, but prints a
    validation report to [stderr] and aborts the program if it is invalid. See
    [llvm::verifyModule]. *)
external assert_valid_module : Llvm.llmodule -> unit
                             = "llvm_assert_valid_module"

(** [verify_function f] returns if the function [f] is valid, but prints a
    validation report to [stderr] and aborts the program if it is invalid. See
    [llvm::verifyFunction]. *)
external assert_valid_function : Llvm.llvalue -> unit
                               = "llvm_assert_valid_function"

(** [view_function_cfg f] opens up a ghostscript window displaying the CFG of
    the current function with the code for each basic block inside.
    See [llvm::Function::viewCFG]. *)
external view_function_cfg : Llvm.llvalue -> unit = "llvm_view_function_cfg"

(** [view_function_cfg_only f] works just like [view_function_cfg], but does not
    include the contents of basic blocks into the nodes.
    See [llvm::Function::viewCFGOnly]. *)
external view_function_cfg_only : Llvm.llvalue -> unit
                                = "llvm_view_function_cfg_only"

(** [get_callgraph m] gets the call graph of module [m]. See [llvm::CallGraph(Module M)] *)
val get_callgraph :  Llvm.llmodule -> Llvm.llcallgraph 

(** [dispose_callgraph cg] frees the [cg] memory. See [llvm::~CallGraph] *)
val dispose_callgraph : Llvm.llcallgraph -> unit

(** [fold_callees cg f func int] applies [f] to the functions called by [func]. It skips external nodes *)
val fold_callees : Llvm.llcallgraph -> ('a -> Llvm.llvalue -> 'a) -> Llvm.llvalue -> 'a -> 'a 


(** [get_callee_numrefs cg func] gets the number of incoming edges in [cg] to [func]. 
* It includes external nodes. See [llvm::CallGraphNode::getNumReferences]*)
val get_callee_numrefs : Llvm.llcallgraph -> Llvm.llvalue -> int 

(** [get_callee_size cg func] returns the number of functions called by [func]. To be used
* as a bound for [git_ith_callee]. See [llvm:::CallGraphNode::size]. Unless you need external
* nodes use [fold_callees] instead.*)
val get_callee_size : Llvm.llcallgraph -> Llvm.llvalue -> int 

(** [get_ith_callee cg func i] gets the [i]th function called by [func]. Unless you need external
* nodes use [fold_callees] instead. See [llvm::CallGraphNode::operator[]] and [llvm::CallGraphNode::getFunction]
* The option is empty if getFunction() returns a null pointer which is the case for external nodes.*)
val get_ith_callee : Llvm.llcallgraph -> Llvm.llvalue -> int -> Llvm.llvalue option

