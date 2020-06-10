open Oni_Core;

type model;

let initial: model;

[@deriving show]
type command =
  | FormatDocument;

[@deriving show]
type msg =
  | Command(command)
  | DocumentFormatterAvailable({
      handle: int,
      selector: Exthost.DocumentSelector.t,
      displayName: string,
    })
  | EditsReceived({
      sessionId: int,
      edits: list(Vim.Edit.t),
    })
  | EditRequestFailed({
      sessionId: int,
      msg: string,
    })
  | EditCompleted;

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

let update:
  (
    ~maybeBuffer: option(Oni_Core.Buffer.t),
    ~extHostClient: Exthost.Client.t,
    model,
    msg
  ) =>
  (model, outmsg);

// COMMANDS

module Commands: {let formatDocument: Command.t(msg);};

// CONTRIBUTIONS

module Contributions: {let commands: list(Command.t(msg));};
