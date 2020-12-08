// MODEL

type model;

let initial: model;

let recordingMacro: model => option(char);

let subMode: model => Vim.SubMode.t;

// MSG

[@deriving show]
type msg =
  | ModeChanged({
      mode: [@opaque] Vim.Mode.t,
      subMode: [@opaque] Vim.SubMode.t,
      effects: list(Vim.Effect.t),
    })
  | PasteCompleted({mode: [@opaque] Vim.Mode.t})
  | Pasted(string)
  | SettingChanged(Vim.Setting.t)
  | MacroRecordingStarted({register: char})
  | MacroRecordingStopped;

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | SettingsChanged
  | ModeDidChange({
      mode: Vim.Mode.t,
      effects: list(Vim.Effect.t),
    });

// UPDATE

let update: (msg, model) => (model, outmsg);

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

module Configuration: {
  type resolver = string => option(Vim.Setting.value);

  let resolver: model => resolver;
};
