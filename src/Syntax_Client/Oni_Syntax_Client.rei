/*
 Oni_Syntax_Client.rei

 A client for Oni_Syntax_Server - a separate process for managing syntax highlights
 */

open EditorCoreTypes;
open Oni_Core;

open Oni_Syntax;
module Protocol = Oni_Syntax.Protocol;
module ServerToClient = Protocol.ServerToClient;

type t;

let start:
  (
    ~parentPid: string=?,
    ~executablePath: string=?,
    ~onConnected: unit => unit=?,
    ~onClose: int => unit=?,
    ~onHighlights: (~bufferId: int, ~tokens: list(Protocol.TokenUpdate.t)) =>
                   unit,
    ~onHealthCheckResult: bool => unit,
    Exthost.LanguageInfo.t,
    Setup.t
  ) =>
  result(t, string);

let startHighlightingBuffer:
  (
    ~bufferId: int,
    ~filetype: string,
    ~visibleRanges: list(Range.t),
    ~lines: array(string),
    t
  ) =>
  unit;
let stopHighlightingBuffer: (~bufferId: int, t) => unit;
let notifyBufferUpdate: (~bufferUpdate: BufferUpdate.t, t) => unit;
let notifyBufferVisibilityChanged:
  (~bufferId: int, ~ranges: list(Range.t), t) => unit;

let notifyThemeChanged: (t, TokenTheme.t) => unit;
let notifyTreeSitterChanged: (~useTreeSitter: bool, t) => unit;
let healthCheck: t => unit;
let close: t => unit;

module Testing: {
  let simulateReadException: t => unit;
  let simulateMessageException: t => unit;
};
