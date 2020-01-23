(* This file has been adapted from camlp5:
 * https://github.com/camlp5/camlp5/blob/master/ocaml_src/lib/diff.mli
 *)

(*
 * Copyright (c) 2007-2017, INRIA (Institut National de Recherches en
 * Informatique et Automatique). All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of INRIA, nor the names of its contributors may be
 *       used to endorse or promote products derived from this software without
 *       specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY INRIA AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INRIA AND
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *)

(** Differences between two arrays. *)

val f : 'a array -> 'a array -> bool array * bool array;;
(** [Diff.f a1 a2] returns a couple of two arrays of booleans [(d1, d2)].
      [d1] has the same size as [a1].
      [d2] has the same size as [a2].
      [d1.(i)] is [True] if [a1.(i)] has no corresponding value in [a2].
      [d2.(i)] is [True] if [a2.(i)] has no corresponding value in [a1].
      [d1] and [d2] have the same number of values equal to [False].
    Can be used to write the [diff] program (comparison of two files),
    the input arrays being the array of lines of each file.
    Can be used also to compare two strings (they must have been exploded
    into arrays of chars), or two DNA strings, and so on.
*)