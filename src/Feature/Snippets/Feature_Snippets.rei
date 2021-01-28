open Oni_Core;
open EditorCoreTypes;

// Placeholder until full snippet support: Break snippet at first placeholder
let snippetToInsert: (~snippet: string) => string;

[@deriving show]
type msg;

type model;

let initial: model;

type outmsg =
  | Effect(Isolinear.Effect.t(msg))
  | ErrorMessage(string)
  | SetCursors(list(BytePosition.t))
  | SetSelections(list(ByteRange.t))
  | Nothing;

module Snippet: {
  type raw;

  let parse: string => result(raw, string);

  type t;

  let resolve:
    (
      ~prefix: string,
      ~postfix: string,
      ~indentationSettings: IndentationSettings.t,
      raw
    ) =>
    t;
};

module Session: {
  type t;

  let startLine: t => EditorCoreTypes.LineNumber.t;
  let stopLine: t => EditorCoreTypes.LineNumber.t;
};

let session: model => option(Session.t);

let isActive: model => bool;

let modeChanged: (~mode: Vim.Mode.t, model) => model;

// module Session: {
//   type t;

//   let start: (
//     ~editorId: int,
//     ~buffer: Buffer.t,
//     ~position: BytePosition.t,
//     ~snippet: Snippet.t
//   ) => Isolinear.Effect.t(msg);
// }

let update:
  (
    ~maybeBuffer: option(Buffer.t),
    ~editorId: int,
    ~cursorPosition: BytePosition.t,
    msg,
    model
  ) =>
  (model, outmsg);

module Contributions: {
  let commands: list(Command.t(msg));
  let contextKeys: model => WhenExpr.ContextKeys.t;
  let keybindings: list(Feature_Input.Schema.keybinding);
};
