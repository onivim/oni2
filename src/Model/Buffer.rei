/*
 * Buffer.rei
 *
 * In-memory text buffer representation
 */

module Time = Revery.Time;

open Oni_Core;
open Oni_Core.Types;

type t;

type highlight = VisualRange.t;

let show: t => string;

let ofLines: array(string) => t;
let ofMetadata: Vim.BufferMetadata.t => t;

let getFilePath: t => option(string);
let setFilePath: (option(string), t) => t;
let getFileType: t => option(string);
let setFileType: (option(string), t) => t;
let getLine: (t, int) => string;
let getLineLength: (t, int) => int;
let getLines: t => array(string);

let getHighlights: (Time.t, t) => list(BufferHighlights.highlight);
let addHighlights: (Time.t, list(BufferHighlights.highlight), t) => t;

let getVersion: t => int;
let setVersion: (int, t) => t;

let getUri: t => Uri.t;
let getId: t => int;
let getNumberOfLines: t => int;
let isModified: t => bool;
let isSyntaxHighlightingEnabled: t => bool;

let isIndentationSet: t => bool;
let setIndentation: (IndentationSettings.t, t) => t;
let getIndentation: t => option(IndentationSettings.t);
let setModified: (bool, t) => t;
let disableSyntaxHighlighting: t => t;

let update: (t, BufferUpdate.t) => t;

let empty: t;
