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
    | ConfigurationChanged([@opaque] Configuration.t)
    | ThemeChanged([@opaque] TokenTheme.t)
    | VisibleRangesChanged(
        [@opaque] list((int /* buffer id */, list(Range.t))),
      )
    | Close;
};
