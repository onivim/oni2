open Oni_Core;
open EditorInput;

type outmsg =
  | Nothing
  | DebugInputShown
  | MapParseError({
      fromKeys: string,
      toKeys: string,
      error: string,
    });

[@deriving show]
type command;

// MODEL

module Schema: {
  type keybinding;

  // Bind a key to a command
  let bind:
    (~key: string, ~command: string, ~condition: WhenExpr.t) => keybinding;

  // Clear all bindings for a key
  let clear: (~key: string) => keybinding;

  // Remap a key -> to another key
  let remap:
    (~fromKeys: string, ~toKeys: string, ~condition: WhenExpr.t) => keybinding;

  let mapCommand: (~f: string => string, keybinding) => keybinding;

  type resolvedKeybinding;

  let resolve: keybinding => result(resolvedKeybinding, string);
};

[@deriving show]
type msg;

module Msg: {
  let keybindingsUpdated: list(Schema.resolvedKeybinding) => msg;
  let vimMap: Vim.Mapping.t => msg;
  let vimUnmap: (Vim.Mapping.mode, option(string)) => msg;
};

type model;

let initial: list(Schema.keybinding) => model;

type execute =
  | NamedCommand(string)
  | VimExCommand(string);

type effect =
  | Execute(execute)
  | Text(string)
  | Unhandled(KeyPress.t)
  | RemapRecursionLimitHit;

let keyDown:
  (
    ~config: Config.resolver,
    ~key: KeyPress.t,
    ~context: WhenExpr.ContextKeys.t,
    ~time: Revery.Time.t,
    model
  ) =>
  (model, list(effect));

let text:
  (~text: string, ~time: Revery.Time.t, model) => (model, list(effect));

let candidates:
  (~config: Config.resolver, ~context: WhenExpr.ContextKeys.t, model) =>
  list((EditorInput.Matcher.t, execute));

let commandToAvailableBindings:
  (
    ~command: string,
    ~config: Config.resolver,
    ~context: WhenExpr.ContextKeys.t,
    model
  ) =>
  list(list(EditorInput.KeyPress.t));

let keyPressToString: EditorInput.KeyPress.t => string;

let consumedKeys: model => list(EditorInput.KeyPress.t);

let keyUp:
  (
    ~config: Config.resolver,
    ~key: KeyPress.t,
    ~context: WhenExpr.ContextKeys.t,
    model
  ) =>
  (model, list(effect));

type uniqueId;

let addKeyBinding:
  (~binding: Schema.resolvedKeybinding, model) => (model, uniqueId);

let remove: (uniqueId, model) => model;

let enable: model => model;
let disable: model => model;

// UPDATE

let update: (msg, model) => (model, outmsg);

// SUBSCRIPTION

let sub: model => Isolinear.Sub.t(msg);

// CONTRIBUTIONS

module Contributions: {
  let commands: list(Command.t(msg));
  let configuration: list(Config.Schema.spec);
  let contextKeys: model => WhenExpr.ContextKeys.t;
};

// VIEW

module View: {
  module Overlay: {
    let make:
      (~input: model, ~uiFont: UiFont.t, ~bottom: int, ~right: int, unit) =>
      Revery.UI.element;
  };

  module Matcher: {
    let make:
      (~matcher: EditorInput.Matcher.t, ~font: UiFont.t, unit) =>
      Revery.UI.element;
  };
};
