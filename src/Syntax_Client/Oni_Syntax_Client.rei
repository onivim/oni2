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

type connectedCallback = unit => unit;
type closeCallback = int => unit;
type highlightsCallback = list(Protocol.TokenUpdate.t) => unit;

let start:
  (
    ~parentPid: string=?,
    ~executablePath: string=?,
    ~onConnected: connectedCallback=?,
    ~onClose: closeCallback=?,
    ~onHighlights: highlightsCallback,
    ~onHealthCheckResult: bool => unit,
    Oni_Extensions.LanguageInfo.t,
    Setup.t
  ) =>
  result(t, string);

let notifyBufferEnter: (t, int, string, array(string)) => unit;
let notifyBufferLeave: (t, int) => unit;
let notifyThemeChanged: (t, TokenTheme.t) => unit;
let notifyConfigurationChanged: (t, Configuration.t) => unit;
let notifyBufferUpdate: (t, BufferUpdate.t, string) => unit;

let notifyVisibilityChanged: (t, list((int, list(Range.t)))) => unit;
let healthCheck: t => unit;
let close: t => unit;

module Testing: {
  let simulateReadException: t => unit;
  let simulateMessageException: t => unit;
};
