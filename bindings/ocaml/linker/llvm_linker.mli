(*===-- llvm_linker.mli - LLVM OCaml Interface -----------------*- OCaml -*-===*
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 *===----------------------------------------------------------------------===*)

(** Linker.

    This interface provides an OCaml API for LLVM bitcode linker,
    the classes in the Linker library. *)

exception Error of string

(** [link_modules' dst src] links [src] into [dst], raising [Error]
    if the linking fails. The src module is destroyed. *)
val link_modules' : Llvm.llmodule -> Llvm.llmodule -> unit

(** [get_linker dst] creates a linker context used to link into module [dst]
*   see llvm::Linker *)
val get_linker : Llvm.llmodule -> Llvm.lllinker 

(** [linker_dispose lllinker] frees up linker from get_linker *)
val linker_dispose :  Llvm.lllinker -> unit

(** [link_in dst src] links [src] into [dst] *)
val link_in :  Llvm.lllinker -> Llvm.llmodule -> unit

