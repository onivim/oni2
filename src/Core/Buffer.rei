/*
 * Buffer.rei
 *
 * In-memory text buffer representation
 */

type t;

let show: t => string;

let ofLines: array(string) => t;
let ofMetadata: Vim.BufferMetadata.t => t;

let getId: t => int;
let getUri: t => Uri.t;
let getFilePath: t => option(string);
let setFilePath: (option(string), t) => t;
let getFileType: t => option(string);
let setFileType: (option(string), t) => t;
let getLine: (t, int) => string;
let getLineLength: (t, int) => int;
let getLines: t => array(string);
let getNumberOfLines: t => int;

let getVersion: t => int;
let setVersion: (int, t) => t;

let isModified: t => bool;
let setModified: (bool, t) => t;

let isIndentationSet: t => bool;
let setIndentation: (IndentationSettings.t, t) => t;
let getIndentation: t => option(IndentationSettings.t);

let isSyntaxHighlightingEnabled: t => bool;
let disableSyntaxHighlighting: t => t;

let stampLastUsed: t => t;
let getLastUsed: t => float;

let shouldApplyUpdate: (BufferUpdate.t, t) => bool;
let update: (t, BufferUpdate.t) => t;

let empty: t;
