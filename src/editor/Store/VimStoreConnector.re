/*
 * VimStoreConnector.re
 *
 * This module connects vim to the Store:
 * - Translates incoming vim notifications into Actions
 * - Translates Actions into Effects that should run against vim
 */

module Core = Oni_Core;
module Extensions = Oni_Extensions;
module Model = Oni_Model;

module Log = Core.Log;
module Zed_utf8 = Core.ZedBundled;

let start =
    (getState: unit => Model.State.t, getClipboardText, setClipboardText) => {
  let (stream, dispatch) = Isolinear.Stream.create();

  Vim.Clipboard.setProvider(reg => {
    let state = getState();
    let yankConfig =
      Model.Selectors.getActiveConfigurationValue(state, c =>
        c.vimUseSystemClipboard
      );

    let removeWindowsNewLines = s =>
      List.init(String.length(s), String.get(s))
      |> List.filter(c => c != '\r')
      |> List.map(c => String.make(1, c))
      |> String.concat("");

    let splitNewLines = s => String.split_on_char('\n', s) |> Array.of_list;

    let getClipboardValue = () => {
      switch (getClipboardText()) {
      | None => None
      | Some(v) => Some(v |> removeWindowsNewLines |> splitNewLines)
      };
    };

    let starReg = Char.code('*');
    let plusReg = Char.code('+');
    let unnamedReg = 0;

    let shouldPullFromClipboard =
      (reg == starReg || reg == plusReg)  // always for '*' and '+'
      || reg == unnamedReg
      && yankConfig.paste; // or if 'paste' set, but unnamed

    if (shouldPullFromClipboard) {
      getClipboardValue();
    } else {
      None;
    };
  });

  /*let _ =
    // Unhandled escape is called when there is an `<esc>` sent to Vim,
    // but nothing to escape from (ie, in normal mode with no pending operator)
    Vim.onUnhandledEscape(() => {
      print_endline ("Unhandled escape");
    });*/

  let _ =
    Vim.Mode.onChanged(newMode =>
      dispatch(Model.Actions.ChangeMode(newMode))
    );

  let _ =
    Vim.onDirectoryChanged(newDir =>
      dispatch(Model.Actions.OpenExplorer(newDir))
    );

  let _ =
    Vim.onMessage((priority, t, msg) => {
      open Vim.Types;
      let (priorityString, notificationType) =
        switch (priority) {
        | Error => ("ERROR", Model.Actions.Error)
        | Warning => ("WARNING", Model.Actions.Warning)
        | Info => ("INFO", Model.Actions.Info)
        };

      Log.info("Message -" ++ priorityString ++ " [" ++ t ++ "]: " ++ msg);

      dispatch(
        ShowNotification(
          Model.Notification.create(
            ~notificationType,
            ~title="libvim",
            ~message=msg,
            (),
          ),
        ),
      );
    });

  let _ =
    Vim.onYank(({lines, register, operator, _}) => {
      let state = getState();
      let yankConfig =
        Model.Selectors.getActiveConfigurationValue(state, c =>
          c.vimUseSystemClipboard
        );
      let allYanks = yankConfig.yank;
      let allDeletes = yankConfig.delete;
      let isClipboardRegister = register == '*' || register == '+';
      let shouldPropagateToClipboard =
        isClipboardRegister
        || operator == Vim.Yank.Yank
        && allYanks
        || operator == Vim.Yank.Delete
        && allDeletes;
      if (shouldPropagateToClipboard) {
        let text = String.concat("\n", Array.to_list(lines));
        setClipboardText(text);
      };
    });

  let _ =
    Vim.Buffer.onFilenameChanged(meta => {
      Log.info("Buffer metadata changed: " ++ string_of_int(meta.id));
      let meta = {
        ...meta,
        /*
             Set version to 0 so that a buffer update is processed.
             If not - we'd ignore the first buffer update that came through!
         */
        version: 0,
      };
      dispatch(Model.Actions.BufferEnter(meta));
    });

  let _ =
    Vim.Buffer.onModifiedChanged((id, modified) => {
      Log.info(
        "Buffer metadata changed: "
        ++ string_of_int(id)
        ++ " | "
        ++ string_of_bool(modified),
      );
      dispatch(Model.Actions.BufferSetModified(id, modified));
    });

  let _ =
    Vim.Cursor.onMoved(newPosition => {
      let cursorPos =
        Core.Types.Position.createFromOneBasedIndices(
          newPosition.line,
          newPosition.column + 1,
        );
      dispatch(Model.Actions.CursorMove(cursorPos));

      let buffer = Vim.Buffer.getCurrent();
      let id = Vim.Buffer.getId(buffer);

      let result = Vim.Search.getMatchingPair();
      switch (result) {
      | None => dispatch(Model.Actions.SearchClearMatchingPair(id))
      | Some({line, column}) =>
        dispatch(
          Model.Actions.SearchSetMatchingPair(
            id,
            Core.Types.Position.create(
              OneBasedIndex(newPosition.line),
              ZeroBasedIndex(newPosition.column),
            ),
            Core.Types.Position.create(
              OneBasedIndex(line),
              ZeroBasedIndex(column),
            ),
          ),
        )
      };
    });

  let _ =
    Vim.Search.onStopSearchHighlight(() => {
      let buffer = Vim.Buffer.getCurrent();
      let id = Vim.Buffer.getId(buffer);
      dispatch(Model.Actions.SearchClearHighlights(id));
    });

  let _ =
    Vim.onQuit((quitType, force) =>
      switch (quitType) {
      | QuitAll => dispatch(Quit(force))
      | QuitOne(buf) => dispatch(QuitBuffer(buf, force))
      }
    );

  let _ =
    Vim.Visual.onRangeChanged(vr => {
      open Vim.Range;
      open Vim.VisualRange;

      let {visualType, range} = vr;
      let {startPos, endPos} = range;
      let startColumn = startPos.column + 1;
      let endColumn = endPos.column + 1;
      let vr =
        Core.VisualRange.create(
          ~startLine=startPos.line,
          ~startColumn,
          ~endLine=endPos.line,
          ~endColumn,
          ~mode=visualType,
          (),
        );
      dispatch(SelectionChanged(vr));
    });

  let _ =
    Vim.Window.onSplit((splitType, buf) => {
      /* If buf wasn't specified, use the filepath from the current buffer */
      let buf =
        switch (buf) {
        | "" =>
          switch (Vim.Buffer.getFilename(Vim.Buffer.getCurrent())) {
          | None => ""
          | Some(v) => v
          }
        | v => v
        };

      Log.info("Vim.Window.onSplit: " ++ buf);

      let command =
        switch (splitType) {
        | Vim.Types.Vertical =>
          Model.Actions.OpenFileByPath(buf, Some(Model.WindowTree.Vertical))
        | Vim.Types.Horizontal =>
          Model.Actions.OpenFileByPath(
            buf,
            Some(Model.WindowTree.Horizontal),
          )
        | Vim.Types.TabPage => Model.Actions.OpenFileByPath(buf, None)
        };
      dispatch(command);
    });

  let _ =
    Vim.Window.onMovement((movementType, _count) => {
      Log.info("Vim.Window.onMovement");
      let currentState = getState();

      let move = moveFunc => {
        let windowId = moveFunc(currentState.windowManager);
        let maybeEditorGroupId =
          Model.WindowTree.getEditorGroupIdFromSplitId(
            windowId,
            currentState.windowManager.windowTree,
          );

        switch (maybeEditorGroupId) {
        | Some(editorGroupId) =>
          dispatch(Model.Actions.WindowSetActive(windowId, editorGroupId))
        | None => ()
        };
      };

      switch (movementType) {
      | FullLeft
      | OneLeft => move(Model.WindowManager.moveLeft)
      | FullRight
      | OneRight => move(Model.WindowManager.moveRight)
      | FullDown
      | OneDown => move(Model.WindowManager.moveDown)
      | FullUp
      | OneUp => move(Model.WindowManager.moveUp)
      | RotateDownwards =>
        dispatch(Model.Actions.Command("view.rotateForward"))
      | RotateUpwards =>
        dispatch(Model.Actions.Command("view.rotateBackward"))
      | _ => move(windowManager => windowManager.activeWindowId)
      };
    });

  let _ =
    Vim.Buffer.onEnter(buf => {
      let meta = {
        ...Vim.BufferMetadata.ofBuffer(buf),
        /*
             Set version to 0 so that a buffer update is processed.
             If not - we'd ignore the first buffer update that came through!
         */
        version: 0,
      };
      dispatch(Model.Actions.BufferEnter(meta));
    });

  let _ =
    Vim.Buffer.onUpdate(update => {
      open Vim.BufferUpdate;
      Log.info("Vim - Buffer update: " ++ string_of_int(update.id));
      open Core.Types;
      let bu =
        Core.Types.BufferUpdate.create(
          ~id=update.id,
          ~startLine=Index.OneBasedIndex(update.startLine),
          ~endLine=Index.OneBasedIndex(update.endLine),
          ~lines=update.lines,
          ~version=update.version,
          (),
        );

      dispatch(Model.Actions.BufferUpdate(bu));
    });

  let _ =
    Vim.CommandLine.onEnter(c =>
      dispatch(Model.Actions.CommandlineShow(c.cmdType))
    );

  let lastCompletionMeet = ref(None);
  let isCompleting = ref(false);

  let checkCommandLineCompletions = () => {
    Log.info("VimStoreConnector::checkCommandLineCompletions");
    let completions = Vim.CommandLine.getCompletions() |> Array.to_list;
    Log.info(
      "VimStoreConnector::checkCommandLineCompletions - got "
      ++ string_of_int(List.length(completions))
      ++ " completions.",
    );
    dispatch(Model.Actions.WildmenuShow(completions));
  };

  let _ =
    Vim.CommandLine.onUpdate(c => {
      dispatch(Model.Actions.CommandlineUpdate(c));

      let cmdlineType = Vim.CommandLine.getType();
      switch (cmdlineType) {
      | Ex =>
        ();
        let text =
          switch (Vim.CommandLine.getText()) {
          | Some(v) => v
          | None => ""
          };
        let position = Vim.CommandLine.getPosition();
        let meet = Core.Utility.getCommandLineCompletionsMeet(text, position);
        lastCompletionMeet := meet;

        isCompleting^ ? () : checkCommandLineCompletions();
      | SearchForward
      | SearchReverse =>
        let highlights = Vim.Search.getHighlights();

        let sameLineFilter = (range: Vim.Range.t) =>
          range.startPos.line == range.endPos.line;

        let buffer = Vim.Buffer.getCurrent();
        let id = Vim.Buffer.getId(buffer);

        let toOniRange = (range: Vim.Range.t) =>
          Core.Range.create(
            ~startLine=OneBasedIndex(range.startPos.line),
            ~startCharacter=ZeroBasedIndex(range.startPos.column),
            ~endLine=OneBasedIndex(range.endPos.line),
            ~endCharacter=ZeroBasedIndex(range.endPos.column),
            (),
          );

        let highlightList =
          highlights
          |> Array.to_list
          |> List.filter(sameLineFilter)
          |> List.map(toOniRange);
        dispatch(SearchSetHighlights(id, highlightList));
      | _ => ()
      };
    });

  let _ =
    Vim.CommandLine.onLeave(() => {
      lastCompletionMeet := None;
      isCompleting := false;
      dispatch(Model.Actions.CommandlineHide);
    });

  let _ =
    Vim.Window.onTopLineChanged(t => {
      Log.info("onTopLineChanged: " ++ string_of_int(t));
      dispatch(Model.Actions.EditorScrollToLine(t - 1));
    });

  let _ =
    Vim.Window.onLeftColumnChanged(t => {
      Log.info("onLeftColumnChanged: " ++ string_of_int(t));
      dispatch(Model.Actions.EditorScrollToColumn(t));
    });

  let hasInitialized = ref(false);
  let initEffect =
    Isolinear.Effect.create(~name="vim.init", () => {
      Vim.init();
      let _ = Vim.command("e untitled");
      hasInitialized := true;
    });

  /* TODO: Move to init */
  /* let metadata = Vim.Buffer.getCurrent() */
  /* |> Vim.BufferMetadata.ofBuffer; */
  /* dispatch(Model.Actions.BufferEnter(metadata)); */

  let currentBufferId: ref(option(int)) = ref(None);
  let currentEditorId: ref(option(int)) = ref(None);

  let inputEffect = key =>
    Isolinear.Effect.create(~name="vim.input", ()
      /* TODO: Fix these keypaths in libvim to not be blocking */
      =>
        if (Oni_Input.Filter.filter(key)) {
          Log.debug("VimStoreConnector - handling key: " ++ key);
          Vim.input(key);
          Log.debug("VimStoreConnector - handled key: " ++ key);
        }
      );

  let openFileByPathEffect = (filePath, dir) =>
    Isolinear.Effect.create(~name="vim.openFileByPath", () => {
      /* If a split was requested, create that first! */
      switch (dir) {
      | Some(direction) =>
        let eg = Model.EditorGroup.create();
        dispatch(Model.Actions.EditorGroupAdd(eg));

        let split =
          Model.WindowTree.createSplit(~editorGroupId=eg.editorGroupId, ());

        dispatch(Model.Actions.AddSplit(direction, split));
      | None => ()
      };

      let buffer = Vim.Buffer.openFile(filePath);
      let metadata = Vim.BufferMetadata.ofBuffer(buffer);

      /*
       * If we're splitting, make sure a BufferEnter event gets dispatched.
       * (This wouldn't happen if we're splitting the same buffer we're already at)
       */
      switch (dir) {
      | Some(_) => dispatch(Model.Actions.BufferEnter(metadata))
      | None => ()
      };
    });

  let applyCompletionEffect = completion =>
    Isolinear.Effect.create(~name="vim.applyCommandlineCompletion", () =>
      Core.Utility.(
        switch (lastCompletionMeet^) {
        | None => ()
        | Some({position, _}) =>
          isCompleting := true;
          let currentPos = ref(Vim.CommandLine.getPosition());
          while (currentPos^ > position) {
            Vim.input("<bs>");
            currentPos := Vim.CommandLine.getPosition();
          };

          let completion = Core.Utility.trimTrailingSlash(completion);
          String.iter(c => Vim.input(String.make(1, c)), completion);
          isCompleting := false;
        }
      )
    );

  let synchronizeIndentationEffect = (indentation: Core.IndentationSettings.t) =>
    Isolinear.Effect.create(~name="vim.setIndentation", () => {
      let insertSpaces =
        switch (indentation.mode) {
        | Tabs => false
        | Spaces => true
        };

      Vim.Options.setTabSize(indentation.size);
      Vim.Options.setInsertSpaces(insertSpaces);
    });

  /**
   synchronizeEditorEffect checks the current state of the app:
   - open buffer
   - open editor

   If it is changed from the last time we 'synchronized', we
   push those changes to vim and record the latest state.

   This allows us to keep the buffer management in Onivim 2,
   and treat vim as an entity for manipulating a singular buffer.
   */
  let synchronizeEditorEffect = state =>
    Isolinear.Effect.create(~name="vim.synchronizeEditor", () =>
      switch (hasInitialized^) {
      | false => ()
      | true =>
        let editorGroup = Model.Selectors.getActiveEditorGroup(state);
        let editor = Model.Selectors.getActiveEditor(editorGroup);

        /* If the editor / buffer in Onivim changed,
         * let libvim know about it and set it as the current buffer */
        let editorBuffer = Model.Selectors.getActiveBuffer(state);
        switch (editorBuffer, currentBufferId^) {
        | (Some(editorBuffer), Some(v)) =>
          let id = Model.Buffer.getId(editorBuffer);
          if (id != v) {
            let buf = Vim.Buffer.getById(id);
            switch (buf) {
            | None => ()
            | Some(v) => Vim.Buffer.setCurrent(v)
            };
          };
        | (Some(editorBuffer), _) =>
          let id = Model.Buffer.getId(editorBuffer);
          let buf = Vim.Buffer.getById(id);
          switch (buf) {
          | None => ()
          | Some(v) => Vim.Buffer.setCurrent(v)
          };
        | _ => ()
        };

        let synchronizeWindowMetrics =
            (editor: Model.Editor.t, editorGroup: Model.EditorGroup.t) => {
          let vimWidth = Vim.Window.getWidth();
          let vimHeight = Vim.Window.getHeight();

          let (lines, columns) =
            Model.Editor.getLinesAndColumns(editor, editorGroup.metrics);

          if (columns != vimWidth) {
            Vim.Window.setWidth(columns);
          };

          if (lines != vimHeight) {
            Vim.Window.setHeight(lines);
          };
        };

        /* Update the window metrics for the editor */
        /* This synchronizes the window width / height with libvim's model */
        switch (editor, editorGroup) {
        | (Some(e), Some(v)) => synchronizeWindowMetrics(e, v)
        | _ => ()
        };

        /* Update the cursor position and the scroll (top line, left column) -
         * ensure these are in sync with libvim's model */
        let synchronizeCursorAndScroll = (editor: Model.Editor.t) => {
          Vim.Cursor.setPosition(
            Core.Types.Index.toInt1(editor.cursorPosition.line),
            Core.Types.Index.toInt0(editor.cursorPosition.character),
          );
          Vim.Window.setTopLeft(
            Core.Types.Index.toInt1(editor.lastTopLine),
            Core.Types.Index.toInt0(editor.lastLeftCol),
          );
        };

        /* If the editor changed, we need to synchronize various aspects, like the cursor position, topline, and leftcol */
        switch (editor, currentEditorId^) {
        | (Some(e), Some(v)) when e.editorId != v =>
          synchronizeCursorAndScroll(e);
          currentEditorId := Some(e.editorId);
        | (Some(e), None) =>
          synchronizeCursorAndScroll(e);
          currentEditorId := Some(e.editorId);
        | _ => ()
        };
      }
    );

  let pasteIntoEditorAction =
    Isolinear.Effect.create(~name="vim.clipboardPaste", () =>
      if (Vim.Mode.getCurrent() == Vim.Types.Insert) {
        switch (getClipboardText()) {
        | Some(text) =>
          Vim.command("set paste");
          Zed_utf8.iter(s => Vim.input(Zed_utf8.singleton(s)), text);

          Vim.command("set nopaste");
        | None => ()
        };
      }
    );

  let copyActiveFilepathToClipboardEffect =
    Isolinear.Effect.create(~name="vim.copyActiveFilepathToClipboard", () =>
      switch (Vim.Buffer.getCurrent() |> Vim.Buffer.getFilename) {
      | Some(filename) => setClipboardText(filename)
      | None => ()
      }
    );

  let updater = (state: Model.State.t, action) => {
    switch (action) {
    | Model.Actions.Command("editor.action.clipboardPasteAction") => (
        state,
        pasteIntoEditorAction,
      )
    | Model.Actions.WildmenuNext =>
      let eff =
        switch (Model.Wildmenu.getSelectedItem(state.wildmenu)) {
        | None => Isolinear.Effect.none
        | Some(v) => applyCompletionEffect(v)
        };
      (state, eff);
    | Model.Actions.WildmenuPrevious =>
      let eff =
        switch (Model.Wildmenu.getSelectedItem(state.wildmenu)) {
        | None => Isolinear.Effect.none
        | Some(v) => applyCompletionEffect(v)
        };
      (state, eff);
    | Model.Actions.Init => (state, initEffect)
    | Model.Actions.OpenFileByPath(path, direction) => (
        state,
        openFileByPathEffect(path, direction),
      )
    | Model.Actions.BufferEnter(_)
    | Model.Actions.SetEditorFont(_)
    | Model.Actions.WindowSetActive(_, _)
    | Model.Actions.EditorGroupSetSize(_, _) => (
        state,
        synchronizeEditorEffect(state),
      )
    | Model.Actions.BufferSetIndentation(_, indent) => (
        state,
        synchronizeIndentationEffect(indent),
      )
    | Model.Actions.ViewSetActiveEditor(_) => (
        state,
        synchronizeEditorEffect(state),
      )
    | Model.Actions.ViewCloseEditor(_) => (
        state,
        synchronizeEditorEffect(state),
      )
    | Model.Actions.KeyboardInput(s) => (state, inputEffect(s))
    | Model.Actions.CopyActiveFilepathToClipboard => (
        state,
        copyActiveFilepathToClipboardEffect,
      )
    | _ => (state, Isolinear.Effect.none)
    };
  };

  (updater, stream);
};
