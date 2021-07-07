open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;
module Log = (val Log.withNamespace("Oni2.Service.Vim"));

module EditConverter = {
  open Oni_Core;

  let textToArray = (~removeTrailingNewLine) =>
    fun
    | None => [|""|]
    | Some(text) =>
      text
      |> Utility.StringEx.removeWindowsNewLines
      |> (
        text =>
          removeTrailingNewLine
            ? Utility.StringEx.removeTrailingNewLine(text) : text
      )
      |> Utility.StringEx.splitNewLines;

  let extHostSingleEditToVimEdit =
      (~buffer, edit: Exthost.Edit.SingleEditOperation.t) => {
    let range = edit.range |> Exthost.OneBasedRange.toRange;

    let bufferRange = Oni_Core.Buffer.characterRange(buffer);

    // We need to remove the trailing new line in some specific circumstances:
    // - The edit covers the entire buffer
    // AND
    // - The buffer has no trailing new line

    // Some formatters, like prettier or the ocaml formatter, send a single,
    // uber-edit - with all the formatted lines in a buffer. In addition,
    // they _might_ add a trailing new line (and if we don't handle it -
    // will hit bugs like #3320).

    // Other formatters, like Python or clangd, send line-by-line formats -
    // it's important to not ignore the trailing new-lines for line-by-line
    // edits, because otherwise we'll hit bugs like #3288

    let removeTrailingNewLine =
      !Oni_Core.Buffer.hasTrailingNewLine(buffer)
      && CharacterRange.containsRange(~query=bufferRange, range);
    (
      {range, text: textToArray(~removeTrailingNewLine, edit.text)}: Vim.Edit.t
    );
  };
};

let forceReload = () =>
  Isolinear.Effect.create(~name="vim.discardChanges", () =>
    ignore(Vim.command("e!"): (Vim.Context.t, list(Vim.Effect.t)))
  );

let forceOverwrite = () =>
  Isolinear.Effect.create(~name="vim.forceOverwrite", () =>
    ignore(Vim.command("w!"): (Vim.Context.t, list(Vim.Effect.t)))
  );

let reload = () =>
  Isolinear.Effect.create(~name="vim.reload", () => {
    ignore(Vim.command("e"): (Vim.Context.t, list(Vim.Effect.t)))
  });

let saveAllAndQuit = () =>
  Isolinear.Effect.create(~name="lifecycle.saveAllAndQuit", () =>
    ignore(Vim.command("xa"): (Vim.Context.t, list(Vim.Effect.t)))
  );

let quitAll = () =>
  Isolinear.Effect.create(~name="lifecycle.saveAllAndQuit", () =>
    ignore(Vim.command("qa!"): (Vim.Context.t, list(Vim.Effect.t)))
  );

module Effects = {
  let paste = (~context=?, ~toMsg, text) => {
    Isolinear.Effect.createWithDispatch(~name="vim.clipboardPaste", dispatch => {
      let context =
        switch (context) {
        | None => Vim.Context.current()
        | Some(ctx) => ctx
        };
      let mode = context.mode;
      let isCmdLineMode = Vim.Mode.isCommandLine(mode);
      let isInsertMode = Vim.Mode.isInsert(mode);
      let isSelectMode = Vim.Mode.isSelect(mode);
      let isNormalMode = Vim.Mode.isNormal(mode);
      let isVisualMode = Vim.Mode.isVisual(mode);

      if (isInsertMode || isCmdLineMode || isSelectMode) {
        if (!isCmdLineMode) {
          Vim.command("set paste") |> ignore;
        };

        Log.infof(m => m("Pasting: %s", text));
        let (latestContext: Vim.Context.t, _effects) =
          Vim.input(~context, text);

        if (!isCmdLineMode) {
          Vim.command("set nopaste") |> ignore;
          dispatch(toMsg(latestContext.mode));
        };
      } else if (isVisualMode || isNormalMode) {
        let (latestContext: Vim.Context.t, _effects) =
          Vim.input(~context, "\"*p");
        dispatch(toMsg(latestContext.mode));
      };
    });
  };

  let getRegisterValue = (~toMsg, char) =>
    Isolinear.Effect.createWithDispatch(~name="vim.getRegisterValue", dispatch => {
      let result = Vim.Registers.get(~register=char);
      dispatch(toMsg(result));
    });

  let applyEdits =
      (
        ~shouldAdjustCursors,
        ~bufferId: int,
        ~version: int,
        ~edits: list(Vim.Edit.t),
        toMsg: result(unit, string) => 'msg,
      ) => {
    Isolinear.Effect.createWithDispatch(~name="vim.applyEdits", dispatch => {
      Log.infof(m => m("Trying to apply edits to buffer: %d", bufferId));
      let maybeBuffer = Vim.Buffer.getById(bufferId);
      let result =
        switch (maybeBuffer) {
        | None =>
          Error("No buffer found with id: " ++ string_of_int(bufferId))
        | Some(buffer) =>
          let bufferVersion = Vim.Buffer.getVersion(buffer);
          if (bufferVersion < version) {
            Error(
              Printf.sprintf(
                "Expected buffer version %d, got %d",
                bufferVersion,
                version,
              ),
            );
          } else {
            Vim.Buffer.applyEdits(~shouldAdjustCursors, ~edits, buffer);
          };
        };

      result |> toMsg |> dispatch;
    });
  };

  let setLines =
      (
        ~shouldAdjustCursors,
        ~bufferId: int,
        ~start=?,
        ~stop=?,
        ~lines: array(string),
        toMsg: result(unit, string) => 'msg,
      ) => {
    Isolinear.Effect.createWithDispatch(~name="vim.setLines", dispatch => {
      Log.infof(m => m("Trying to set lines for buffer: %d", bufferId));
      let maybeBuffer = Vim.Buffer.getById(bufferId);
      let result =
        switch (maybeBuffer) {
        | None =>
          Error("No buffer found with id: " ++ string_of_int(bufferId))
        | Some(buffer) =>
          Vim.Buffer.setLines(
            ~shouldAdjustCursors,
            ~undoable=true,
            ~start?,
            ~stop?,
            ~lines,
            buffer,
          );
          Ok();
        };

      result |> toMsg |> dispatch;
    });
  };

  let adjustBytePositionForEdit =
      (bytePosition: BytePosition.t, edit: Vim.Edit.t) => {
    let editStartLine =
      EditorCoreTypes.LineNumber.toZeroBased(edit.range.start.line);
    let editStopLine =
      EditorCoreTypes.LineNumber.toZeroBased(edit.range.stop.line);
    if (editStopLine
        <= EditorCoreTypes.LineNumber.toZeroBased(bytePosition.line)) {
      let originalLines = editStopLine - editStartLine + 1;
      let newLines = Array.length(edit.text);
      let deltaLines = newLines - originalLines;
      BytePosition.{
        line: EditorCoreTypes.LineNumber.(bytePosition.line + deltaLines),
        byte: bytePosition.byte,
      };
    } else {
      bytePosition;
    };
  };

  let adjustModeForEdit = (mode: Vim.Mode.t, edit: Vim.Edit.t) => {
    Vim.Mode.(
      switch (mode) {
      | Normal({cursor}) =>
        Normal({cursor: adjustBytePositionForEdit(cursor, edit)})
      | Insert({cursors}) =>
        Insert({
          cursors:
            cursors
            |> List.map(cursor => adjustBytePositionForEdit(cursor, edit)),
        })
      | Replace({cursor}) =>
        Replace({cursor: adjustBytePositionForEdit(cursor, edit)})
      | CommandLine({commandType, text, commandCursor, cursor}) =>
        CommandLine({
          commandType,
          text,
          commandCursor,
          cursor: adjustBytePositionForEdit(cursor, edit),
        })
      | Operator({cursor, pending}) =>
        Operator({cursor: adjustBytePositionForEdit(cursor, edit), pending})
      | Visual(_) as vis => vis
      | Select(_) as select => select
      }
    );
  };

  let adjustModeForEdits = (mode: Vim.Mode.t, edits: list(Vim.Edit.t)) => {
    List.fold_left(adjustModeForEdit, mode, edits);
  };

  let applyCompletion =
      (
        ~cursor: CharacterPosition.t,
        ~replaceSpan: CharacterSpan.t,
        ~insertText,
        ~toMsg,
        ~additionalEdits,
      ) =>
    Isolinear.Effect.createWithDispatch(~name="applyCompletion", dispatch => {
      let overwriteBefore =
        CharacterIndex.toInt(cursor.character)
        - CharacterIndex.toInt(replaceSpan.start);

      let overwriteAfter =
        max(
          0,
          CharacterIndex.toInt(replaceSpan.stop)
          - CharacterIndex.toInt(cursor.character),
        );

      let _: Vim.Context.t = VimEx.repeatKey(overwriteAfter, "<DEL>");
      let _: Vim.Context.t = VimEx.repeatKey(overwriteBefore, "<BS>");
      let ({mode, _}: Vim.Context.t, _effects) =
        VimEx.inputString(insertText);

      let buffer = Vim.Buffer.getCurrent();
      let mode' =
        if (additionalEdits != []) {
          // We're manually adjusting the cursors here...
          Vim.Buffer.applyEdits(
            ~shouldAdjustCursors=false,
            ~edits=additionalEdits,
            buffer,
          )
          |> Result.map(() => {adjustModeForEdits(mode, additionalEdits)})
          |> ResultEx.value(~default=mode);
        } else {
          mode;
        };

      dispatch(toMsg(mode'));
    });

  let loadBuffer = (~filePath: string, toMsg) => {
    Isolinear.Effect.createWithDispatch(~name="loadBuffer", dispatch => {
      let currentBuffer = Vim.Buffer.getCurrent();

      let newBuffer = Vim.Buffer.openFile(filePath);

      // Revert to previous buffer
      Vim.Buffer.setCurrent(currentBuffer);

      dispatch(toMsg(~bufferId=Vim.Buffer.getId(newBuffer)));
    });
  };

  let saveAll = toMsg => {
    Isolinear.Effect.createWithDispatch(~name="saveAll", dispatch => {
      let (_: Vim.Context.t, _effects: list(Vim.Effect.t)) =
        Vim.command("wa!");
      dispatch(toMsg());
    });
  };

  let save = (~bufferId, toMsg) => {
    Isolinear.Effect.createWithDispatch(~name="saveBuffer", dispatch => {
      let context = {...Vim.Context.current(), bufferId};
      let (_: Vim.Context.t, _effects: list(Vim.Effect.t)) =
        Vim.command(~context, "w!");
      dispatch(toMsg());
    });
  };
};

module Sub = {
  module EvalSubscription =
    Isolinear.Sub.Make({
      type state = unit;
      type params = string;
      type msg = result(string, string);
      let name = "Vim.Sub.Eval";
      let id = params => params;
      let init = (~params, ~dispatch) => {
        Vim.eval(params) |> dispatch;
      };
      let update = (~params as _, ~state as _, ~dispatch as _) => {
        ();
      };
      let dispose = (~params as _, ~state as _) => {
        ();
      };
    });
  let eval = (~toMsg, expression) => {
    EvalSubscription.create(expression) |> Isolinear.Sub.map(toMsg);
  };

  type searchHighlightParams = {
    bufferId: int,
    version: int,
    searchPattern: string,
    topVisibleLine: EditorCoreTypes.LineNumber.t,
    bottomVisibleLine: EditorCoreTypes.LineNumber.t,
  };

  module SearchHighlightsSubscription =
    Isolinear.Sub.Make({
      type state = {
        dispose: unit => unit,
        lastTopLine: EditorCoreTypes.LineNumber.t,
        lastBottomLine: EditorCoreTypes.LineNumber.t,
        lastVersion: int,
      };
      type nonrec params = searchHighlightParams;
      type msg = array(ByteRange.t);
      let name = "Vim.Sub.SearchHighlights";
      let id = ({bufferId, searchPattern, _}) => {
        Printf.sprintf("%d/%s", bufferId, searchPattern);
      };

      let queueSearchHighlights = (~debounceTime, params, dispatch) => {
        Revery.Tick.timeout(
          ~name="Service_Vim.Timeout.searchHighlights",
          _ => {
            let maybeBuffer = Vim.Buffer.getById(params.bufferId);
            switch (maybeBuffer) {
            | None => ()
            | Some(buffer) =>
              let topLine =
                params.topVisibleLine |> EditorCoreTypes.LineNumber.toOneBased;
              let bottomLine =
                params.bottomVisibleLine
                |> EditorCoreTypes.LineNumber.toOneBased;
              Log.infof(m =>
                m(
                  "Getting highlights for buffer: %d (%d-%d)",
                  params.bufferId,
                  topLine,
                  bottomLine,
                )
              );
              let ranges =
                Vim.Search.getHighlightsInRange(buffer, topLine, bottomLine);
              dispatch(ranges);
            };
          },
          debounceTime,
        );
      };

      let init = (~params, ~dispatch) => {
        // Two scenarios to accomodate:
        // - Typing / refining search
        // - Re-querying highlights for buffer updates

        // Refining search needs to be fast, because we want to show new highlights
        // real-time as the user types. However, re-querying highlights on buffer update
        // doesn't need to be as quick, because in the general case we can just shift the existing highlights.
        let debounceTime = Constants.highPriorityDebounceTime;

        let dispose = queueSearchHighlights(~debounceTime, params, dispatch);
        {
          dispose,
          lastTopLine: params.topVisibleLine,
          lastBottomLine: params.bottomVisibleLine,
          lastVersion: params.version,
        };
      };

      let update = (~params, ~state, ~dispatch) =>
        if (!
              EditorCoreTypes.LineNumber.equals(
                params.topVisibleLine,
                state.lastTopLine,
              )
            || !
                 EditorCoreTypes.LineNumber.equals(
                   params.bottomVisibleLine,
                   state.lastBottomLine,
                 )
            || state.lastVersion != params.version) {
          let debounceTime = Constants.mediumPriorityDebounceTime;
          state.dispose();
          let dispose =
            queueSearchHighlights(~debounceTime, params, dispatch);
          {
            dispose,
            lastTopLine: params.topVisibleLine,
            lastBottomLine: params.bottomVisibleLine,
            lastVersion: params.version,
          };
        } else {
          state;
        };

      let dispose = (~params as _, ~state) => {
        state.dispose();
      };
    });

  let searchHighlights =
      (
        ~bufferId,
        ~version,
        ~searchPattern,
        ~topVisibleLine,
        ~bottomVisibleLine,
        toMsg,
      ) =>
    SearchHighlightsSubscription.create({
      bufferId,
      version,
      searchPattern,
      topVisibleLine,
      bottomVisibleLine,
    })
    |> Isolinear.Sub.map(msg => toMsg(msg));
};
