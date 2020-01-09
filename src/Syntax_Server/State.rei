/*
 State.rei

 State modelled for the syntax server
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Syntax;

module Ext = Oni_Extensions;

type t;
let empty: t;

type logFunc = string => unit;

let initialize: (~log: logFunc, Ext.LanguageInfo.t, Setup.t, t) => t;

let getVisibleBuffers: t => list(int);
let getVisibleHighlighters: t => list(NativeSyntaxHighlights.t);
let getActiveHighlighters: t => list(NativeSyntaxHighlights.t);

let getKeywordsForScope: (~scope: string, t) => list(string);

let anyPendingWork: t => bool;

let bufferEnter: (int, t) => t;
let bufferUpdate:
  (~scope: string, ~bufferUpdate: BufferUpdate.t, ~lines: array(string), t) =>
  t;

let updateTheme: (TokenTheme.t, t) => t;
let updateConfiguration: (Configuration.t, t) => t;

/* [updateVisibility(bufferRangeList)] sets the ranges that are visible per-buffer, which allows syntax highlight to only run necessary work */
let updateVisibility: (list((int, list(Range.t))), t) => t;

let doPendingWork: t => t;

let getTokenUpdates: t => list(Protocol.TokenUpdate.t);
let clearTokenUpdates: t => t;
