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
  type keybinding = {
    key: string,
    command: string,
    condition: WhenExpr.t,
  };

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
    model
  ) =>
  (model, list(effect));

let text: (~text: string, model) => (model, list(effect));
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

// UPDATE

let update: (msg, model) => (model, outmsg);

module Contributions: {
  let commands: list(Command.t(msg));
  let configuration: list(Config.Schema.spec);
};
