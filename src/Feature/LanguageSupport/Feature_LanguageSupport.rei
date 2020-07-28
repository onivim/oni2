open EditorCoreTypes;
open Oni_Core;

type model;

let initial: model;

[@deriving show]
type msg;

module Msg: {
  let exthost: Exthost.Msg.LanguageFeatures.msg => msg;
  let keyPressed: string => msg;
  let pasted: string => msg;
};

type outmsg =
  | Nothing
  | OpenFile({
      filePath: string,
      location: option(Location.t),
    })
  | Effect(Isolinear.Effect.t(msg));

let update:
  (
    ~maybeBuffer: option(Oni_Core.Buffer.t),
    ~cursorLocation: Location.t,
    ~client: Exthost.Client.t,
    msg,
    model
  ) =>
  (model, outmsg);

let isFocused: model => bool;

let sub:
  (
    ~isInsertMode: bool,
    ~activeBuffer: Oni_Core.Buffer.t,
    ~activePosition: Location.t,
    ~visibleBuffers: list(Oni_Core.Buffer.t),
    ~client: Exthost.Client.t,
    model
  ) =>
  Isolinear.Sub.t(msg);

module Contributions: {
  let commands: list(Command.t(msg));
  let contextKeys: WhenExpr.ContextKeys.Schema.t(model);
  let keybindings: list(Oni_Input.Keybindings.keybinding);
};

module Definition: {
  let get: (~bufferId: int, model) => option(Exthost.DefinitionLink.t);

  let getAt:
    (~bufferId: int, ~range: Range.t, model) =>
    option(Exthost.DefinitionLink.t);

  let isAvailable: (~bufferId: int, model) => bool;
};

module DocumentHighlights: {
  let getByLine: (~bufferId: int, ~line: int, model) => list(Range.t);

  let getLinesWithHighlight: (~bufferId: int, model) => list(int);
};

// TODO: Remove
module Completions = Completions;
module CompletionItem = CompletionItem;
module CompletionMeet = CompletionMeet;
module Diagnostic = Diagnostic;
module Diagnostics = Diagnostics;
module LanguageFeatures = LanguageFeatures;
