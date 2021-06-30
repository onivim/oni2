open EditorCoreTypes;

// MODEL

type model;

let initial: model;

let recordingMacro: model => option(char);

let subMode: model => Vim.SubMode.t;

let experimentalViml: model => list(string);

type vimUseSystemClipboard = {
  yank: bool,
  delete: bool,
  paste: bool,
};

let useSystemClipboard: model => vimUseSystemClipboard;

// MSG

[@deriving show]
type msg;

module Msg: {
  let modeChanged:
    (
      ~allowAnimation: bool,
      ~subMode: Vim.SubMode.t,
      ~mode: Vim.Mode.t,
      ~effects: list(Vim.Effect.t)
    ) =>
    msg;
  let output: (~cmd: string, ~output: option(string), ~isSilent: bool) => msg;
  let pasted: string => msg;
  let settingChanged: (~setting: Vim.Setting.t) => msg;
};

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | SettingsChanged({
      name: string,
      value: Vim.Setting.value,
    })
  | ModeDidChange({
      allowAnimation: bool,
      mode: Vim.Mode.t,
      effects: list(Vim.Effect.t),
    })
  | Output({
      cmd: string,
      output: option(string),
    });

// UPDATE

let update: (~vimContext: Vim.Context.t, msg, model) => (model, outmsg);

let getSearchHighlightsByLine:
  (~bufferId: int, ~line: LineNumber.t, model) => list(ByteRange.t);

let moveMarkers:
  (
    ~newBuffer: Oni_Core.Buffer.t,
    ~markerUpdate: Oni_Core.MarkerUpdate.t,
    model
  ) =>
  model;

// SUBSCRIPTION

let sub:
  (
    ~buffer: Oni_Core.Buffer.t,
    ~topVisibleLine: LineNumber.t,
    ~bottomVisibleLine: LineNumber.t,
    model
  ) =>
  Isolinear.Sub.t(msg);

module CommandLine: {let getCompletionMeet: string => option(int);};

module Effects: {
  let applyCompletion:
    (
      ~cursor: EditorCoreTypes.CharacterPosition.t,
      ~replaceSpan: EditorCoreTypes.CharacterSpan.t,
      ~insertText: string,
      ~additionalEdits: list(Vim.Edit.t)
    ) =>
    Isolinear.Effect.t(msg);

  let save: (~bufferId: int) => Isolinear.Effect.t(msg);

  let setTerminalLines:
    (~editorId: int, ~bufferId: int, array(string)) =>
    Isolinear.Effect.t(msg);
};

// CONFIGURATION

let configurationChanged: (~config: Oni_Core.Config.resolver, model) => model;

module Configuration: {
  type resolver = string => option(Vim.Setting.value);

  let resolver: model => resolver;
};

module Contributions: {
  let keybindings: list(Feature_Input.Schema.keybinding);
  let configuration: list(Oni_Core.Config.Schema.spec);

  let commands: list(Oni_Core.Command.t(msg));
};
