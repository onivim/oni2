open Oni_Core;
open EditorInput;

// TODO: Move to Service_Input
module ReveryKeyConverter = ReveryKeyConverter;

type outmsg =
  | Nothing
  | DebugInputShown
  | ErrorNotifications(list(string))
  | MapParseError({
      fromKeys: string,
      toKeys: string,
      error: string,
    })
  | TimedOut;

[@deriving show]
type command;

// MODEL

module Schema: {
  type keybinding;

  // Bind a key to a command
  let bind:
    (~key: string, ~command: string, ~condition: WhenExpr.t) => keybinding;

  // Bind a key to a command, with arguments
  let bindWithArgs:
    (
      ~arguments: Yojson.Safe.t,
      ~key: string,
      ~command: string,
      ~condition: WhenExpr.t
    ) =>
    keybinding;

  // Clear all bindings for a key
  let clear: (~key: string) => keybinding;

  // Remap a key -> to another key
  let remap:
    (
      ~allowRecursive: bool,
      ~fromKeys: string,
      ~toKeys: string,
      ~condition: WhenExpr.t
    ) =>
    keybinding;

  let mapCommand: (~f: string => string, keybinding) => keybinding;

  type resolvedKeybinding;

  let resolvedToString: resolvedKeybinding => string;

  let resolve: keybinding => result(resolvedKeybinding, string);
};

// LOADER

module KeybindingsLoader: {
  type t;

  let none: t;

  let file: FpExp.t(FpExp.absolute) => t;
};

[@deriving show]
type msg;

module Msg: {
  let keybindingsUpdated: list(Schema.resolvedKeybinding) => msg;
  let vimMap: Vim.Mapping.t => msg;
  let vimUnmap: (Vim.Mapping.mode, option(string)) => msg;
};

type model;

let initial: (~loader: KeybindingsLoader.t, list(Schema.keybinding)) => model;

type execute =
  | NamedCommand({
      command: string,
      arguments: Yojson.Safe.t,
    })
  | VimExCommand(string);

type effect =
  | Execute(execute)
  | Text(string)
  | Unhandled({
      key: KeyCandidate.t,
      isProducedByRemap: bool,
    })
  | RemapRecursionLimitHit;

let keyDown:
  (
    ~config: Config.resolver,
    ~scancode: int,
    ~key: KeyCandidate.t,
    ~context: WhenExpr.ContextKeys.t,
    ~time: Revery.Time.t,
    model
  ) =>
  (model, list(effect));

let timeout:
  (~context: WhenExpr.ContextKeys.t, model) => (model, list(effect));

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
let keyCandidateToString: EditorInput.KeyCandidate.t => string;

let consumedKeys: model => list(EditorInput.KeyCandidate.t);

let keyUp:
  (
    ~config: Config.resolver,
    ~scancode: int,
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

let notifyFileSaved: (FpExp.t(FpExp.absolute), model) => model;

// UPDATE

let update: (msg, model) => (model, outmsg);

// SUBSCRIPTION

let sub: (~config: Config.resolver, model) => Isolinear.Sub.t(msg);

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
