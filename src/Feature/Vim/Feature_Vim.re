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
type command =
  | MoveSelectionUpward
  | MoveSelectionDownward
  | CopySelectionUpward
  | CopySelectionDownward;

[@deriving show]
type msg =
  | Command(command)
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
  let applyCompletion = (~cursor, ~replaceSpan, ~insertText, ~additionalEdits) => {
    let toMsg = mode =>
      ModeChanged({
        allowAnimation: true,
        subMode: Vim.SubMode.None,
        mode,
        effects: [],
      });
    Service_Vim.Effects.applyCompletion(
      ~cursor,
      ~replaceSpan,
      ~insertText,
      ~additionalEdits,
      ~toMsg,
    );
  };

  let save = (~bufferId) => {
    Isolinear.Effect.createWithDispatch(
      ~name="Feature_Vim.Effect.save", dispatch => {
      let context = {...Vim.Context.current(), bufferId};

      let (newContext, effects) = Vim.command(~context, "w!");

      dispatch(
        ModeChanged({
          allowAnimation: false,
          mode: newContext.mode,
          subMode: newContext.subMode,
          effects,
        }),
      );
    });
  };
};

let update = (msg, model: model) => {
  switch (msg) {
  | Command(command) =>
    prerr_endline("COMMAND: " ++ show_command(command));
    failwith("Done");

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

module Commands = {
  open Feature_Commands.Schema;

  let moveLinesDown =
    define(
      ~category="Editor",
      ~title="Move lines down",
      "editor.action.moveLinesDownAction",
      Command(MoveSelectionDownward),
    );

  let moveLinesDown =
    define(
      ~title="Move Line Down",
      "editor.action.moveLinesDownAction",
      Command(MoveSelectionDownward),
    );

  let moveLinesUp =
    define(
      ~title="Move Line Up",
      "editor.action.moveLinesUpAction",
      Command(MoveSelectionUpward),
    );

  let copyLinesDown =
    define(
      ~category="Copy Line Down",
      "editor.action.copyLinesDownAction",
      Command(CopySelectionDownward),
    );

  let copyLinesUp =
    define(
      ~category="Copy Line Up",
      "editor.action.copyLinesUpAction",
      Command(CopySelectionUpward),
    );
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

  let editCondition =
    WhenExpr.parse(
      "normalMode || visualMode || insertMode && editorTextFocus",
    );

  let moveLineDownwardJ =
    bind(
      ~key="<A-j>",
      ~command=Commands.moveLinesDown.id,
      ~condition=editCondition,
    );

  let moveLineDownwardArrow =
    bind(
      ~key="<A-Down>",
      ~command=Commands.moveLinesDown.id,
      ~condition=editCondition,
    );

  let moveLineUpwardK =
    bind(
      ~key="<A-k>",
      ~command=Commands.moveLinesUp.id,
      ~condition=editCondition,
    );

  let moveLineUpwardArrow =
    bind(
      ~key="<A-Up>",
      ~command=Commands.moveLinesUp.id,
      ~condition=editCondition,
    );

  let copyLineDownwardJ =
    bind(
      ~key="<A-S-j>",
      ~command=Commands.copyLinesDown.id,
      ~condition=editCondition,
    );

  let copyLineDownwardArrow =
    bind(
      ~key="<A-S-Down>",
      ~command=Commands.copyLinesDown.id,
      ~condition=editCondition,
    );

  let copyLineUpwardK =
    bind(
      ~key="<A-S-k>",
      ~command=Commands.copyLinesUp.id,
      ~condition=editCondition,
    );

  let copyLineUpwardArrow =
    bind(
      ~key="<A-S-Up>",
      ~command=Commands.copyLinesUp.id,
      ~condition=editCondition,
    );
};

module Contributions = {
  let commands =
    Commands.[moveLinesDown, moveLinesUp, copyLinesDown, copyLinesUp];
  let keybindings =
    Keybindings.[
      // Remaps
      controlSquareBracketRemap,
      // Bindings
      moveLineDownwardJ,
      moveLineDownwardArrow,
      moveLineUpwardK,
      moveLineUpwardArrow,
      copyLineDownwardJ,
      copyLineDownwardArrow,
      copyLineUpwardArrow,
      copyLineUpwardK,
    ];

  let configuration = Configuration.[experimentalViml.spec];
};
