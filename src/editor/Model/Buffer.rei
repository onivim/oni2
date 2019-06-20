/*
 * Buffer.rei
 *
 * In-memory text buffer representation
 */

open Oni_Core;
open Oni_Core.Types;

type t;

let show: t => string;

let ofLines: array(string) => t;
let ofMetadata: Vim.BufferMetadata.t => t;

let getLine: (t, int) => string;
let getLineLength: (t, int) => int;

let getMetadata: t => Vim.BufferMetadata.t;
let getUri: t => Uri.t;
let getId: t => int;

let getNumberOfLines: t => int;

let update: (t, BufferUpdate.t) => t;
let updateMetadata: (Vim.BufferMetadata.t, t) => t;

let empty: t;
