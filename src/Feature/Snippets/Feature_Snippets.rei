open Oni_Core;
open EditorCoreTypes;

[@deriving show]
type msg;

module Msg: {let insert: (~snippet: string) => msg;};

type model;

let initial: model;

type outmsg =
  | Effect(Isolinear.Effect.t(msg))
  | ErrorMessage(string)
  | SetCursors(list(BytePosition.t))
  | SetSelections(list(ByteRange.t))
  | ShowPicker(list(Service_Snippets.SnippetWithMetadata.t))
  | Nothing;

module Session: {
  type t;

  let startLine: t => EditorCoreTypes.LineNumber.t;
  let stopLine: t => EditorCoreTypes.LineNumber.t;

  let editorId: t => int;
};

let session: model => option(Session.t);

let isActive: model => bool;

let modeChanged: (~mode: Vim.Mode.t, model) => model;

let update:
  (
    ~resolverFactory: (unit, string) => option(string),
    ~selections: list(VisualRange.t),
    ~maybeBuffer: option(Buffer.t),
    ~editorId: int,
    ~cursorPosition: BytePosition.t,
    ~extensions: Feature_Extensions.model,
    msg,
    model
  ) =>
  (model, outmsg);

module Effects: {
  let insertSnippet:
    (~replaceRange: option(ByteRange.t), ~snippet: string) =>
    Isolinear.Effect.t(msg);
};

module Contributions: {
  let commands: list(Command.t(msg));
  let contextKeys: model => WhenExpr.ContextKeys.t;
  let keybindings: list(Feature_Input.Schema.keybinding);
};
