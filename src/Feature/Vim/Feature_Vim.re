open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;

// MODEL

type model = {
  mode: Vim.Mode.t,
  settings: StringMap.t(Vim.Setting.value),
  recordingMacro: option(char),
};

let initial = {
  mode: Vim.Mode.Normal,
  settings: StringMap.empty,
  recordingMacro: None,
};

let mode = ({mode, _}) => mode;

let recordingMacro = ({recordingMacro, _}) => recordingMacro;

// MSG

[@deriving show]
type msg =
  | ModeChanged([@opaque] Vim.Mode.t)
  | PasteCompleted({cursors: [@opaque] list(BytePosition.t)})
  | Pasted(string)
  | SettingChanged(Vim.Setting.t)
  | MacroRecordingStarted({register: char})
  | MacroRecordingStopped;

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | CursorsUpdated(list(BytePosition.t));

let update = (msg, model: model) => {
  switch (msg) {
  | ModeChanged(mode) => ({...model, mode}, Nothing)
  | Pasted(text) =>
    let eff =
      Service_Vim.Effects.paste(
        ~toMsg=cursors => PasteCompleted({cursors: cursors}),
        text,
      );
    (model, Effect(eff));
  | PasteCompleted({cursors}) => (model, CursorsUpdated(cursors))
  | SettingChanged(({fullName, value, _}: Vim.Setting.t)) => (
      {...model, settings: model.settings |> StringMap.add(fullName, value)},
      Nothing,
    )
  | MacroRecordingStarted({register}) => (
      {...model, recordingMacro: Some(register)},
      Nothing,
    )
  | MacroRecordingStopped => ({...model, recordingMacro: None}, Nothing)
  };
};

module CommandLine = {
  let getCompletionMeet = commandLine => {
    let len = String.length(commandLine);

    if (len == 0) {
      None;
    } else {
      String.index_opt(commandLine, ' ')
      |> Option.map(idx => idx + 1)  // Advance past space
      |> OptionEx.or_(Some(0));
    };
  };

  let%test "empty command line returns None" = {
    getCompletionMeet("") == None;
  };

  let%test "meet before command" = {
    getCompletionMeet("vsp") == Some(0);
  };

  let%test "meet after command" = {
    getCompletionMeet("vsp ") == Some(4);
  };

  let%test "meet with a path, no spaces" = {
    getCompletionMeet("vsp /path/") == Some(4);
  };

  let%test "meet with a path, spaces" = {
    getCompletionMeet("vsp /path with spaces/") == Some(4);
  };
};

module Configuration = {
  type resolver = string => option(Vim.Setting.value);

  let resolver = ({settings, _}, settingName) => {
    settings |> StringMap.find_opt(settingName);
  };
};
