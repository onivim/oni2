/*
 * Protocol represents the communication protocol
 * between the Syntax_client and Syntax_server
 */

open EditorCoreTypes;
open Oni_Core;

module Ext = Oni_Extensions;

let pidToNamedPipe = pid => {
  Exthost.(
    {
      let name = Printf.sprintf("syntax-client-%s", pid);
      name |> NamedPipe.create |> NamedPipe.toString;
    }
  );
};

module TokenUpdate = {
  type t = {
    line: int,
    tokenColors: list(ColorizedToken.t),
  };

  let show = tokenUpdate => {
    Printf.sprintf(
      "Line: %d token count: %d",
      tokenUpdate.line,
      List.length(tokenUpdate.tokenColors),
    );
  };

  let create = (~line, tokenColors) => {line, tokenColors};
};

module ServerToClient = {
  [@deriving show({with_path: false})]
  type t =
    | Initialized
    | TokenUpdate({
        bufferId: int,
        tokens: [@opaque] list(TokenUpdate.t),
      })
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
    | BufferStartHighlighting({
        bufferId: int,
        filetype: string,
        lines: [@opaque] array(string),
        visibleRanges: [@opaque] list(Range.t),
      })
    | BufferStopHighlighting(int)
    | BufferVisibilityChanged({
        bufferId: int,
        ranges: [@opaque] list(Range.t),
      })
    | BufferUpdate([@opaque] Oni_Core.BufferUpdate.t)
    | UseTreeSitter(bool)
    | ThemeChanged([@opaque] TokenTheme.t)
    | RunHealthCheck
    | Close
    // Debug
    | SimulateMessageException
    | SimulateReadException;
};
