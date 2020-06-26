open EditorCoreTypes;
open Oni_Core;

type model;

let initial: model;

[@deriving show]
type command =
  | FormatDocument
  | FormatRange;

[@deriving show]
type msg =
  | Command(command)
  | DocumentFormatterAvailable({
      handle: int,
      selector: Exthost.DocumentSelector.t,
      displayName: string,
    })
  | RangeFormatterAvailable({
      handle: int,
      selector: Exthost.DocumentSelector.t,
      displayName: string,
    })
  | EditsReceived({
      displayName: string,
      sessionId: int,
      edits: list(Vim.Edit.t),
    })
  | EditRequestFailed({
      sessionId: int,
      msg: string,
    })
  | EditCompleted({
      editCount: int,
      displayName: string,
    });

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | FormattingApplied({
      displayName: string,
      editCount: int,
    })
  | FormatError(string);

let update:
  (
    ~languageConfiguration: Oni_Core.LanguageConfiguration.t,
    ~configuration: Oni_Core.Configuration.t,
    ~maybeSelection: option(Range.t),
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
