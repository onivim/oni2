/*
 State.rei

 State modelled for the syntax server
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Syntax;

type t;
let empty: t;

type logFunc = string => unit;

let initialize: (~log: logFunc, Exthost.LanguageInfo.t, Setup.t, t) => t;

let getVisibleBuffers: t => list(int);
let getVisibleHighlighters: t => list(NativeSyntaxHighlights.t);
let getActiveHighlighters: t => list(NativeSyntaxHighlights.t);

let anyPendingWork: t => bool;

let bufferEnter:
  (
    ~bufferId: int,
    ~filetype: string,
    ~lines: array(string),
    ~visibleRanges: list(Range.t),
    t
  ) =>
  t;
let bufferUpdate: (~bufferUpdate: BufferUpdate.t, t) => result(t, string);
let bufferLeave: (~bufferId: int, t) => t;

let updateTheme: (TokenTheme.t, t) => t;
let setUseTreeSitter: (bool, t) => t;

let updateBufferVisibility: (~bufferId: int, ~ranges: list(Range.t), t) => t;

let doPendingWork: t => t;

let getTokenUpdates: t => list((int, list(Protocol.TokenUpdate.t)));
let clearTokenUpdates: t => t;
