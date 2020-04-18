/*
 * Protocol represents the communication protocol
 * between the Syntax_client and Syntax_server
 */

open EditorCoreTypes;
open Oni_Core;
module Ext = Oni_Extensions;

module TokenUpdate = {
  type t = {
    bufferId: int,
    line: int,
    tokenColors: list(ColorizedToken.t),
  };

  let show = tokenUpdate => {
    Printf.sprintf(
      "Buffer id: %d line: %d token count: %d",
      tokenUpdate.bufferId,
      tokenUpdate.line,
      List.length(tokenUpdate.tokenColors),
    );
  };

  let create = (~bufferId, ~line, tokenColors) => {
    bufferId,
    line,
    tokenColors,
  };
};

module ServerToClient = {
  [@deriving show({with_path: false})]
  type t =
    | Initialized
    | TokenUpdate([@opaque] list(TokenUpdate.t))
    | HealthCheckPass(bool)
    | EchoReply(string)
    | Log(string)
    | Closing;
};

module ClientToServer = {
  [@deriving show({with_path: false})]
  type t =
    | Echo(string)
    | Initialize([@opaque] Ext.LanguageInfo.t, Setup.t)
    | BufferEnter(int, string)
    | BufferLeave(int)
    | BufferUpdate(
        [@opaque] Oni_Core.BufferUpdate.t,
        [@opaque] array(string),
        string,
      )
    | UseTreeSitter(bool)
    | ThemeChanged([@opaque] TokenTheme.t)
    | RunHealthCheck
    | VisibleRangesChanged(
        [@opaque] list((int /* buffer id */, list(Range.t))),
      )
    | Close
    // Debug
    | SimulateMessageException
    | SimulateReadException;
};
