open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;

module Log = (val Oni_Core.Log.withNamespace("Feature_Vim"));

// MODEL

type vimUseSystemClipboard = {
  yank: bool,
  delete: bool,
  paste: bool,
};

type model = {
  settings: StringMap.t(Vim.Setting.value),
  recordingMacro: option(char),
  subMode: Vim.SubMode.t,
  searchPattern: option(string),
  searchHighlights: SearchHighlights.t,
  experimentalViml: list(string),
  useSystemClipboard: vimUseSystemClipboard,
};

let initial = {
  settings: StringMap.empty,
  recordingMacro: None,
  subMode: Vim.SubMode.None,
  searchPattern: None,
  searchHighlights: SearchHighlights.initial,
  experimentalViml: [],
  useSystemClipboard: {
    yank: true,
    delete: false,
    paste: false,
  },
};

let useSystemClipboard = ({useSystemClipboard, _}) => useSystemClipboard;

module Configuration = {
  open Config.Schema;

  module Codecs = {
    let vimUseSystemClipboard =
      custom(
        ~encode=
          Json.Encode.(
            {
              (useClipboard: vimUseSystemClipboard) => {
                obj([
                  ("yank", useClipboard.yank |> bool),
                  ("delete", useClipboard.delete |> bool),
                  ("paste", useClipboard.paste |> bool),
                ]);
              };
            }
          ),
        ~decode=
          Json.Decode.(
            {
              let decodeBool =
                bool
                |> map(
                     fun
                     | true => {yank: true, delete: true, paste: true}
                     | false => {yank: false, delete: false, paste: false},
                   );

              let applyString = (prev, str) =>
                switch (String.lowercase_ascii(str)) {
                | "yank" => {...prev, yank: true}
                | "delete" => {...prev, delete: true}
                | "paste" => {...prev, paste: true}
                | _ => prev
                };

              let decodeString =
                string
                |> map(
                     applyString({yank: false, delete: false, paste: false}),
                   );

              let decodeList =
                list(string)
                |> map(
                     List.fold_left(
                       applyString,
                       {yank: false, delete: false, paste: false},
                     ),
                   );

              one_of([
                ("vimUseSystemClipboard.bool", decodeBool),
                ("vimUseSystemClipboard.string", decodeString),
                ("vimUseSystemClipboard.list", decodeList),
              ]);
            }
          ),
      );
  };

  type resolver = string => option(Vim.Setting.value);

  let resolver = ({settings, _}, settingName) => {
    settings |> StringMap.find_opt(settingName);
  };

  let experimentalViml =
    setting("experimental.viml", list(string), ~default=[]);

  let useSystemClipboard =
    setting(
      "vim.useSystemClipboard",
      Codecs.vimUseSystemClipboard,
      ~default={yank: true, delete: false, paste: false},
    );
};

let configurationChanged = (~config, model) => {
  {
    ...model,
    useSystemClipboard: Configuration.useSystemClipboard.get(config),
    experimentalViml: Configuration.experimentalViml.get(config),
  };
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

[@deriving show]
type command =
  | ClearSearchHighlights;

// MSG

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
  | Output({
      cmd: string,
      output: option(string),
      isSilent: bool,
    });

module Msg = {
  let output = (~cmd, ~output, ~isSilent) => Output({cmd, output, isSilent});

  let settingChanged = (~setting) => SettingChanged(setting);

  let modeChanged = (~allowAnimation, ~subMode, ~mode, ~effects) => {
    ModeChanged({allowAnimation, mode, subMode, effects});
  };

  let pasted = text => Pasted(text);
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
  | Vim.Effect.MacroRecordingStarted({register}) => {
      ...model,
      recordingMacro: Some(register),
    }
  | Vim.Effect.MacroRecordingStopped(_) => {...model, recordingMacro: None}
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

  let setTerminalLines = (~editorId as _, ~bufferId, lines) => {
    Isolinear.Effect.createWithDispatch(
      ~name="vim.setTerminalLinesEffect", dispatch => {
      let () =
        bufferId
        |> Vim.Buffer.getById
        |> Option.iter(buf => {
             Vim.Buffer.setModifiable(~modifiable=true, buf);
             Vim.Buffer.setLines(~shouldAdjustCursors=false, ~lines, buf);
             Vim.Buffer.setModifiable(~modifiable=false, buf);
             Vim.Buffer.setReadOnly(~readOnly=true, buf);
           });

      // Clear out previous mode
      let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.key("<esc>");
      let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.key("<esc>");
      // Jump to bottom
      let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.input("g");
      let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.input("g");
      let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.input("G");
      let ({mode, _}: Vim.Context.t, effects) = Vim.input("$");

      // Update the editor, which is the source of truth for cursor position
      dispatch(
        ModeChanged({
          subMode: Vim.SubMode.None,
          allowAnimation: true,
          mode,
          effects,
        }),
      );
    });
  };
};

let update = (~vimContext, msg, model: model) => {
  switch (msg) {
  | Command(ClearSearchHighlights) => (
      {
        ...model,
        searchPattern: None,
        searchHighlights: SearchHighlights.initial,
      },
      Nothing,
    )
  | ModeChanged({allowAnimation, mode, effects, subMode}) => (
      {...model, subMode} |> handleEffects(effects),
      ModeDidChange({allowAnimation, mode, effects}),
    )
  | Pasted(text) =>
    let eff =
      Service_Vim.Effects.paste(
        ~context=vimContext,
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
      SettingsChanged({name: fullName, value}),
    )

  | Output({cmd, output, isSilent}) => (
      model,
      isSilent ? Nothing : Output({cmd, output}),
    )

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

  let clearSearchHighlights =
    define(
      ~category="Search",
      ~title="Clear highlights",
      "vim.clearSearchHighlights",
      Command(ClearSearchHighlights),
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

  // Normalize the Ctrl-^ binding (alternate file) for libvim
  // #3455: The Control+6 key gets sent as, actually, Ctrl-6, which isn't recognized
  let controlCaretRemap =
    remap(
      ~allowRecursive=true,
      ~fromKeys="<C-6>",
      ~toKeys="<C-^>",
      ~condition=WhenExpr.Value(True),
    );

  let normalModeCondition = "editorTextFocus && normalMode" |> WhenExpr.parse;

  let clearSearchHighlights =
    bind(
      ~key="<C-L>",
      ~condition=normalModeCondition,
      ~command=Commands.clearSearchHighlights.id,
    );
};

module Contributions = {
  let keybindings =
    Keybindings.[
      clearSearchHighlights,
      controlSquareBracketRemap,
      controlCaretRemap,
    ];

  let commands = Commands.[clearSearchHighlights];

  let configuration = Configuration.[experimentalViml.spec];
};
