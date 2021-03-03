open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;

module Log = (val Oni_Core.Log.withNamespace("Feature_Vim"));

// MODEL

type model = {
  settings: StringMap.t(Vim.Setting.value),
  recordingMacro: option(char),
  subMode: Vim.SubMode.t,
  searchPattern: option(string),
  searchHighlights: SearchHighlights.t,
  experimentalViml: list(string),
};

let initial = {
  settings: StringMap.empty,
  recordingMacro: None,
  subMode: Vim.SubMode.None,
  searchPattern: None,
  searchHighlights: SearchHighlights.initial,
  experimentalViml: [],
};

module Configuration = {
  type resolver = string => option(Vim.Setting.value);

  let resolver = ({settings, _}, settingName) => {
    settings |> StringMap.find_opt(settingName);
  };

  open Config.Schema;

  let experimentalViml =
    setting("experimental.viml", list(string), ~default=[]);
};

let configurationChanged = (~config, model) => {
  {...model, experimentalViml: Configuration.experimentalViml.get(config)};
};

let recordingMacro = ({recordingMacro, _}) => recordingMacro;

let subMode = ({subMode, _}) => subMode;

let experimentalViml = ({experimentalViml, _}) => experimentalViml;

let moveMarkers = (~newBuffer, ~markerUpdate, model) => {
  ...model,
  searchHighlights:
    SearchHighlights.moveMarkers(
      ~newBuffer,
      ~markerUpdate,
      model.searchHighlights,
    ),
};

// MSG

[@deriving show]
type msg =
  | ModeChanged({
      allowAnimation: bool,
      mode: [@opaque] Vim.Mode.t,
      subMode: [@opaque] Vim.SubMode.t,
      effects: [@opaque] list(Vim.Effect.t),
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

let getSearchHighlightsByLine = (~bufferId, ~line, {searchHighlights, _}) => {
  SearchHighlights.getHighlightsByLine(~bufferId, ~line, searchHighlights);
};

let handleEffect = (model, effect: Vim.Effect.t) => {
  switch (effect) {
  | Vim.Effect.SearchStringChanged(maybeSearchString) => {
      ...model,
      searchPattern: maybeSearchString,
    }
  | Vim.Effect.SearchClearHighlights => {
      ...model,
      searchPattern: None,
      searchHighlights: SearchHighlights.initial,
    }
  | _ => model
  };
};

let handleEffects = (effects, model) => {
  effects |> List.fold_left(handleEffect, model);
};

module Effects = {
  let applyCompletion = (~meetColumn, ~insertText, ~additionalEdits) => {
    let toMsg = mode =>
      ModeChanged({
        allowAnimation: true,
        subMode: Vim.SubMode.None,
        mode,
        effects: [],
      });
    Service_Vim.Effects.applyCompletion(
      ~meetColumn,
      ~insertText,
      ~additionalEdits,
      ~toMsg,
    );
  };
};

let update = (msg, model: model) => {
  switch (msg) {
  | ModeChanged({allowAnimation, mode, effects, subMode}) => (
      {...model, subMode} |> handleEffects(effects),
      ModeDidChange({allowAnimation, mode, effects}),
    )
  | Pasted(text) =>
    let eff =
      Service_Vim.Effects.paste(
        ~toMsg=mode => PasteCompleted({mode: mode}),
        text,
      );
    (model, Effect(eff));
  | PasteCompleted({mode}) => (
      model,
      ModeDidChange({allowAnimation: true, mode, effects: []}),
    )
  | SettingChanged({fullName, value, _}: Vim.Setting.t) => (
      {...model, settings: model.settings |> StringMap.add(fullName, value)},
      SettingsChanged,
    )
  | MacroRecordingStarted({register}) => (
      {...model, recordingMacro: Some(register)},
      Nothing,
    )
  | MacroRecordingStopped => ({...model, recordingMacro: None}, Nothing)

  | Output({cmd, output}) => (model, Output({cmd, output}))

  | SearchHighlightsAvailable({bufferId, highlights}) =>
    let newHighlights =
      highlights |> ArrayEx.filterToList(ByteRange.isSingleLine);
    (
      {
        ...model,
        searchHighlights:
          SearchHighlights.setSearchHighlights(
            bufferId,
            newHighlights,
            model.searchHighlights,
          ),
      },
      Nothing,
    );

  | Noop => (model, Nothing)
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

// SUBSCRIPTION

let sub = (~buffer, ~topVisibleLine, ~bottomVisibleLine, model) => {
  let bufferId = Oni_Core.Buffer.getId(buffer);
  let version = Oni_Core.Buffer.getVersion(buffer);
  model.searchPattern
  |> Option.map(searchPattern => {
       Service_Vim.Sub.searchHighlights(
         ~bufferId,
         ~version,
         ~topVisibleLine,
         ~bottomVisibleLine,
         ~searchPattern,
         ranges => {
         SearchHighlightsAvailable({bufferId, highlights: ranges})
       })
     })
  |> Option.value(~default=Isolinear.Sub.none);
};

module Keybindings = {
  open Feature_Input.Schema;
  let controlSquareBracketRemap =
    remap(
      ~allowRecursive=true,
      ~fromKeys="<C-[>",
      ~toKeys="<ESC>",
      ~condition=WhenExpr.Value(True),
    );
};

module Contributions = {
  let keybindings = Keybindings.[controlSquareBracketRemap];

  let configuration = Configuration.[experimentalViml.spec];
};
