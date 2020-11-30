open Oni_Core;
open Oni_Core.Utility;

// MODEL

type model = {
  settings: StringMap.t(Vim.Setting.value),
  recordingMacro: option(char),
};

let initial = {settings: StringMap.empty, recordingMacro: None};

let recordingMacro = ({recordingMacro, _}) => recordingMacro;

// MSG

[@deriving show]
type msg =
  | ModeChanged({
      mode: [@opaque] Vim.Mode.t,
      effects: [@opaque] list(Vim.Effect.t),
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

let update = (msg, model: model) => {
  switch (msg) {
  | ModeChanged({mode, effects}) =>
    // TODO: Check submode here
    (model, ModeDidChange({mode, effects}))
  | Pasted(text) =>
    let eff =
      Service_Vim.Effects.paste(
        ~toMsg=mode => PasteCompleted({mode: mode}),
        text,
      );
    (model, Effect(eff));
  | PasteCompleted({mode}) => (model, ModeDidChange({mode, effects: []}))
  | SettingChanged(({fullName, value, _}: Vim.Setting.t)) => (
      {...model, settings: model.settings |> StringMap.add(fullName, value)},
      SettingsChanged,
    )
  | MacroRecordingStarted({register}) => (
      {...model, recordingMacro: Some(register)},
      Nothing,
    )
  | MacroRecordingStopped => ({...model, recordingMacro: None}, Nothing)
  };
};

module CommandLine = {
  let getCompletionMeet = commandLine =>
    if (StringEx.isEmpty(commandLine)) {
      None;
    } else {
      StringEx.findUnescapedFromEnd(commandLine, ' ')
      |> OptionEx.or_(Some(0));
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
    getCompletionMeet("vsp /path\\ with\\ spaces/") == Some(4);
  };

  let%test "meet multiple paths" = {
    getCompletionMeet("!cp /path1 /path2") == Some(11);
  };

  let%test "meet multiple paths with spaces" = {
    getCompletionMeet("!cp /path\\ 1 /path\\ 2") == Some(13);
  };
};

module Effects = {
  let applyCompletion = (~meetColumn, ~insertText, ~additionalEdits) => {
    let toMsg = mode => ModeChanged({mode, effects: []});
    Service_Vim.Effects.applyCompletion(
      ~meetColumn,
      ~insertText,
      ~additionalEdits,
      ~toMsg,
    );
  };
};

module Configuration = {
  type resolver = string => option(Vim.Setting.value);

  let resolver = ({settings, _}, settingName) => {
    settings |> StringMap.find_opt(settingName);
  };
};
