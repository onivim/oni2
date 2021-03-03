open EditorCoreTypes;

// MODEL

type model;

let initial: model;

let recordingMacro: model => option(char);

let subMode: model => Vim.SubMode.t;

let experimentalViml: model => list(string);

// MSG

[@deriving show]
type msg =
  | ModeChanged({
      allowAnimation: bool,
      mode: [@opaque] Vim.Mode.t,
      subMode: [@opaque] Vim.SubMode.t,
      effects: list(Vim.Effect.t),
    })
  | PasteCompleted({mode: [@opaque] Vim.Mode.t})
  | Pasted(string)
  | SearchHighlightsAvailable({
      bufferId: int,
      highlights: array(ByteRange.t),
    })
  | SettingChanged(Vim.Setting.t)
  | MacroRecordingStarted({register: char})
  | MacroRecordingStopped
  | Output({
      cmd: string,
      output: option(string),
    })
  | Noop;

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | SettingsChanged
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

let update: (msg, model) => (model, outmsg);

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
      ~meetColumn: EditorCoreTypes.CharacterIndex.t,
      ~insertText: string,
      ~additionalEdits: list(Vim.Edit.t)
    ) =>
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
};
