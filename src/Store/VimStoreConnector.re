/*
 * VimStoreConnector.re
 *
 * This module connects vim to the Store:
 * - Translates incoming vim notifications into Actions
 * - Translates Actions into Effects that should run against vim
 */

module Core = Oni_Core;
module Option = Core.Utility.Option;

open Oni_Model;

module Ext = Oni_Extensions;

module Log = Core.Log;
module Zed_utf8 = Core.ZedBundled;

let start =
    (
      languageInfo: Ext.LanguageInfo.t,
      getState: unit => State.t,
      getClipboardText,
      setClipboardText,
    ) => {
  let (stream, dispatch) = Isolinear.Stream.create();

  Vim.Clipboard.setProvider(reg => {
    let state = getState();
    let yankConfig =
      Selectors.getActiveConfigurationValue(state, c =>
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

  let _ =
    // Unhandled escape is called when there is an `<esc>` sent to Vim,
    // but nothing to escape from (ie, in normal mode with no pending operator)
    Vim.onUnhandledEscape(() => {
      let state = getState();
      if (Notifications.any(state.notifications)) {
        let oldestNotificationId =
          Notifications.getOldestId(state.notifications);
        dispatch(Actions.HideNotification(oldestNotificationId));
      };
    });

  let _ =
    Vim.Mode.onChanged(newMode => dispatch(Actions.ChangeMode(newMode)));

  let _ =
    Vim.onDirectoryChanged(newDir =>
      dispatch(Actions.VimDirectoryChanged(newDir))
    );

  let _ =
    Vim.onMessage((priority, t, msg) => {
      open Vim.Types;
      let (priorityString, notificationType) =
        switch (priority) {
        | Error => ("ERROR", Actions.Error)
        | Warning => ("WARNING", Actions.Warning)
        | Info => ("INFO", Actions.Info)
        };

      Log.info("Message -" ++ priorityString ++ " [" ++ t ++ "]: " ++ msg);

      dispatch(
        ShowNotification(
          Notification.create(
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
        Selectors.getActiveConfigurationValue(state, c =>
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

      let fileType =
        switch (meta.filePath) {
        | Some(v) =>
          Some(Ext.LanguageInfo.getLanguageFromFilePath(languageInfo, v))
        | None => None
        };

      dispatch(Actions.BufferEnter(meta, fileType));
    });

  let _ =
    Vim.Buffer.onModifiedChanged((id, modified) => {
      Log.info(
        "Buffer metadata changed: "
        ++ string_of_int(id)
        ++ " | "
        ++ string_of_bool(modified),
      );
      dispatch(Actions.BufferSetModified(id, modified));
    });

  let _ =
    Vim.Cursor.onMoved(newPosition => {
      let buffer = Vim.Buffer.getCurrent();
      let id = Vim.Buffer.getId(buffer);

      let result = Vim.Search.getMatchingPair();
      switch (result) {
      | None => dispatch(Actions.SearchClearMatchingPair(id))
      | Some({line, column}) =>
        dispatch(
          Actions.SearchSetMatchingPair(
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
      dispatch(Actions.SearchClearHighlights(id));
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
          Actions.OpenFileByPath(buf, Some(WindowTree.Vertical), None)
        | Vim.Types.Horizontal =>
          Actions.OpenFileByPath(buf, Some(WindowTree.Horizontal), None)
        | Vim.Types.TabPage => Actions.OpenFileByPath(buf, None, None)
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
          WindowTree.getEditorGroupIdFromSplitId(
            windowId,
            currentState.windowManager.windowTree,
          );

        switch (maybeEditorGroupId) {
        | Some(editorGroupId) =>
          dispatch(Actions.WindowSetActive(windowId, editorGroupId))
        | None => ()
        };
      };

      switch (movementType) {
      | FullLeft
      | OneLeft => move(WindowManager.moveLeft)
      | FullRight
      | OneRight => move(WindowManager.moveRight)
      | FullDown
      | OneDown => move(WindowManager.moveDown)
      | FullUp
      | OneUp => move(WindowManager.moveUp)
      | RotateDownwards => dispatch(Actions.Command("view.rotateForward"))
      | RotateUpwards => dispatch(Actions.Command("view.rotateBackward"))
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
      let fileType =
        switch (meta.filePath) {
        | Some(v) =>
          Some(Ext.LanguageInfo.getLanguageFromFilePath(languageInfo, v))
        | None => None
        };
      dispatch(Actions.BufferEnter(meta, fileType));
    });

  let _ =
    Vim.Buffer.onUpdate(update => {
      open Vim.BufferUpdate;
      Log.info("Vim - Buffer update: " ++ string_of_int(update.id));
      open Core.Types;
      open State;

      let isFull = update.endLine == (-1);

      let maybeBuffer = Buffers.getBuffer(update.id, getState().buffers);

      // If this is a 'full' update, check if there was a previous buffer.
      // We need to keep track of the previous line count for some
      // buffer synchronization strategies (ie, extension host)
      let endLine =
        if (isFull) {
          maybeBuffer
          |> Option.map(b => Core.Buffer.getNumberOfLines(b) + 1)
          |> Option.value(~default=update.startLine);
        } else {
          update.endLine;
        };

      let bu =
        Core.BufferUpdate.create(
          ~id=update.id,
          ~isFull,
          ~startLine=Index.OneBasedIndex(update.startLine),
          ~endLine=Index.OneBasedIndex(endLine),
          ~lines=update.lines,
          ~version=update.version,
          (),
        );

      // We need to filter out updates that come 'out-of-order'. This shouldn't need to be done
      // at this layer - but there are some bugs with the updates we get from 'reason-libvim'.
      // In particular, with undo / redo, we get multiple updates - a full update (version+2) and
      // an incremental update (version+1) _after_ the full update. If we apply the incremental update
      // after the full update, ignoring the version, we'll get incorrect results.
      //
      // The fix really belongs in reason-libvim - we should always be trust the order we get from the updates,
      // and any of this filtering or manipulation of updates should be handled and tested there.
      let shouldApply =
        Option.map(Core.Buffer.shouldApplyUpdate(bu), maybeBuffer)
        != Some(false);

      if (shouldApply) {
        dispatch(Actions.BufferUpdate(bu));
      } else {
        Log.info(
          "Skipped buffer update at version: "
          ++ string_of_int(update.version),
        );
      };
    });

  let _ =
    Vim.CommandLine.onEnter(c =>
      dispatch(Actions.QuickmenuShow(Wildmenu(c.cmdType)))
    );

  let lastCompletionMeet = ref(None);
  let isCompleting = ref(false);

  let checkCommandLineCompletions = () => {
    Log.info("VimStoreConnector::checkCommandLineCompletions");
    let completions = Vim.CommandLine.getCompletions();
    Log.info(
      "VimStoreConnector::checkCommandLineCompletions - got "
      ++ string_of_int(Array.length(completions))
      ++ " completions.",
    );
    let items =
      Array.map(
        name =>
          Actions.{
            name,
            category: None,
            icon: None,
            command: () => Noop,
            highlight: [],
          },
        completions,
      );
    dispatch(Actions.QuickmenuUpdateFilterProgress(items, Complete));
  };

  let _ =
    Vim.CommandLine.onUpdate(({text, position: cursorPosition, _}) => {
      dispatch(Actions.QuickmenuInput({text, cursorPosition}));

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
      dispatch(Actions.QuickmenuClose);
    });

  let hasInitialized = ref(false);
  let initEffect =
    Isolinear.Effect.create(~name="vim.init", () => {
      Vim.init();
      let _ = Vim.command("e untitled");
      hasInitialized := true;
    });

  let currentBufferId: ref(option(int)) = ref(None);

  let updateActiveEditorCursors = cursors => {
    open Oni_Core.Utility;
    let () =
      getState()
      |> Selectors.getActiveEditorGroup
      |> Selectors.getActiveEditor
      |> Option.map(Editor.getId)
      |> Option.iter(id => {dispatch(Actions.EditorCursorMove(id, cursors))});
    ();
  };

  let inputEffect = key =>
    Isolinear.Effect.create(~name="vim.input", () =>
      if (Oni_Input.Filter.filter(key)) {
        open Oni_Core.Utility;

        // Set cursors based on current editor
        let editor =
          getState()
          |> Selectors.getActiveEditorGroup
          |> Selectors.getActiveEditor;

        let cursors =
          editor
          |> Option.map(Editor.getVimCursors)
          |> Option.value(~default=[]);

        let () =
          editor
          |> Core.Utility.Option.iter(e => {
               let () =
                 getState()
                 |> Selectors.getActiveEditorGroup
                 |> Option.map(EditorGroup.getMetrics)
                 |> Option.iter(metrics => {
                      let topLine = Editor.getTopVisibleLine(e, metrics);
                      let leftCol = Editor.getLeftVisibleColumn(e, metrics);
                      Vim.Window.setTopLeft(topLine, leftCol);
                    });
               ();
             });

        let cursors = Vim.input(~cursors, key);

        let newTopLine = Vim.Window.getTopLine();
        let newLeftColumn = Vim.Window.getLeftColumn();

        let () =
          editor
          |> Option.map(Editor.getId)
          |> Option.iter(id => {
               dispatch(Actions.EditorCursorMove(id, cursors));
               dispatch(Actions.EditorScrollToLine(id, newTopLine - 1));
               dispatch(Actions.EditorScrollToColumn(id, newLeftColumn));
             });
        Log.debug(() => "VimStoreConnector - handled key: " ++ key);
      }
    );

  let openFileByPathEffect = (filePath, dir, location) =>
    Isolinear.Effect.create(~name="vim.openFileByPath", () => {
      open Oni_Core.Utility;
      open Oni_Core.Types;

      /* If a split was requested, create that first! */
      switch (dir) {
      | Some(direction) =>
        let eg = EditorGroup.create();
        dispatch(Actions.EditorGroupAdd(eg));

        let split =
          WindowTree.createSplit(~editorGroupId=eg.editorGroupId, ());

        dispatch(Actions.AddSplit(direction, split));
      | None => ()
      };

      let buffer = Vim.Buffer.openFile(filePath);
      let metadata = Vim.BufferMetadata.ofBuffer(buffer);

      let fileType =
        switch (metadata.filePath) {
        | Some(v) =>
          Some(Ext.LanguageInfo.getLanguageFromFilePath(languageInfo, v))
        | None => None
        };

      let () =
        location
        |> Option.iter((pos: Position.t) => {
             open Position;
             let cursor =
               Vim.Cursor.create(
                 ~line=Index.toInt1(pos.line),
                 ~column=Index.toInt0(pos.character),
                 (),
               );
             let () = updateActiveEditorCursors([cursor]);

             let topLine: int = max(Index.toInt0(pos.line) - 10, 0);

             let () =
               getState()
               |> Selectors.getActiveEditorGroup
               |> Selectors.getActiveEditor
               |> Option.map(Editor.getId)
               |> Option.iter(id =>
                    dispatch(Actions.EditorScrollToLine(id, topLine))
                  );
             ();
           });

      /*
       * If we're splitting, make sure a BufferEnter event gets dispatched.
       * (This wouldn't happen if we're splitting the same buffer we're already at)
       */
      switch (dir) {
      | Some(_) => dispatch(Actions.BufferEnter(metadata, fileType))
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
            let _ = Vim.input(~cursors=[], "<bs>");
            currentPos := Vim.CommandLine.getPosition();
          };

          let completion = Core.Utility.trimTrailingSlash(completion);
          let latestCursors = ref([]);
          String.iter(
            c => {
              latestCursors := Vim.input(~cursors=[], String.make(1, c));
              ();
            },
            completion,
          );
          updateActiveEditorCursors(latestCursors^);
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
  // TODO: Remove remaining 'synchronization'
  let synchronizeEditorEffect = state =>
    Isolinear.Effect.create(~name="vim.synchronizeEditor", () =>
      switch (hasInitialized^) {
      | false => ()
      | true =>
        let editorGroup = Selectors.getActiveEditorGroup(state);
        let editor = Selectors.getActiveEditor(editorGroup);

        /* If the editor / buffer in Onivim changed,
         * let libvim know about it and set it as the current buffer */
        let editorBuffer = Selectors.getActiveBuffer(state);
        switch (editorBuffer, currentBufferId^) {
        | (Some(editorBuffer), Some(v)) =>
          let id = Core.Buffer.getId(editorBuffer);
          if (id != v) {
            let buf = Vim.Buffer.getById(id);
            switch (buf) {
            | None => ()
            | Some(v) => Vim.Buffer.setCurrent(v)
            };
          };
        | (Some(editorBuffer), _) =>
          let id = Core.Buffer.getId(editorBuffer);
          let buf = Vim.Buffer.getById(id);
          switch (buf) {
          | None => ()
          | Some(v) => Vim.Buffer.setCurrent(v)
          };
        | _ => ()
        };

        let synchronizeWindowMetrics =
            (editor: Editor.t, editorGroup: EditorGroup.t) => {
          let vimWidth = Vim.Window.getWidth();
          let vimHeight = Vim.Window.getHeight();

          let (lines, columns) =
            Editor.getLinesAndColumns(editor, editorGroup.metrics);

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
      }
    );

  let pasteIntoEditorAction =
    Isolinear.Effect.create(~name="vim.clipboardPaste", () =>
      if (Vim.Mode.getCurrent() == Vim.Types.Insert) {
        switch (getClipboardText()) {
        | Some(text) =>
          Vim.command("set paste");
          let latestCursors = ref([]);
          Zed_utf8.iter(
            s => {
              latestCursors := Vim.input(~cursors=[], Zed_utf8.singleton(s));
              ();
            },
            text,
          );

          updateActiveEditorCursors(latestCursors^);

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

  let applyCompletion = (state: State.t) =>
    Isolinear.Effect.create(~name="vim.applyCompletion", () => {
      let completions = state.completions;
      let bestMatch = Completions.getBestCompletion(completions);
      let meet = Completions.getMeet(completions);
      Oni_Core.Types.(
        switch (bestMatch, meet) {
        | (Some(completion), Some(meet)) =>
          let cursorPosition = Vim.Cursor.getPosition();
          let delta =
            cursorPosition.column - Index.toInt1(meet.completionMeetColumn);

          let idx = ref(delta);
          while (idx^ >= 0) {
            let _ = Vim.input("<BS>");
            decr(idx);
          };

          let latestCursors = ref([]);
          Zed_utf8.iter(
            s => {
              latestCursors := Vim.input(Zed_utf8.singleton(s));
              ();
            },
            completion.item.completionLabel,
          );
          updateActiveEditorCursors(latestCursors^);
        | _ => ()
        }
      );
    });

  let prevViml = ref([]);
  let synchronizeViml = configuration =>
    Isolinear.Effect.create(~name="vim.synchronizeViml", () => {
      let lines =
        Oni_Core.Configuration.getValue(
          c => c.experimentalVimL,
          configuration,
        );

      if (prevViml^ !== lines) {
        List.iter(
          l => {
            Log.info("Running VimL from config: " ++ l);
            Vim.command(l);
            Log.info("VimL command completed.");
          },
          lines,
        );
        prevViml := lines;
      };
    });

  let undoEffect =
    Isolinear.Effect.create(~name="vim.undo", () => {
      let _ = Vim.input("<esc>");
      let _ = Vim.input("<esc>");
      let cursors = Vim.input("u");
      updateActiveEditorCursors(cursors);
      ();
    });

  let redoEffect =
    Isolinear.Effect.create(~name="vim.redo", () => {
      let _ = Vim.input("<esc>");
      let _ = Vim.input("<esc>");
      let cursors = Vim.input("<c-r>");
      updateActiveEditorCursors(cursors);
      ();
    });

  let updater = (state: State.t, action) => {
    switch (action) {
    | Actions.ConfigurationSet(configuration) => (
        state,
        synchronizeViml(configuration),
      )
    | Actions.Command("editor.action.clipboardPasteAction") => (
        state,
        pasteIntoEditorAction,
      )
    | Actions.Command("insertBestCompletion") => (
        state,
        applyCompletion(state),
      )
    | Actions.Command("undo") => (state, undoEffect)
    | Actions.Command("redo") => (state, redoEffect)
    | Actions.ListFocusUp
    | Actions.ListFocusDown
    | Actions.ListFocus(_) =>
      // IFFY: Depends on the ordering of "updater"s>
      let eff =
        switch (state.quickmenu) {
        | Some({variant: Wildmenu(_), focused: Some(focused), items, _}) =>
          try(applyCompletionEffect(items[focused].name)) {
          | Invalid_argument(_) => Isolinear.Effect.none
          }
        | _ => Isolinear.Effect.none
        };
      (state, eff);

    | Actions.Init => (state, initEffect)
    | Actions.OpenFileByPath(path, direction, location) => (
        state,
        openFileByPathEffect(path, direction, location),
      )
    | Actions.BufferEnter(_)
    | Actions.SetEditorFont(_)
    | Actions.WindowSetActive(_, _)
    | Actions.EditorGroupSetSize(_, _) => (
        state,
        synchronizeEditorEffect(state),
      )
    | Actions.BufferSetIndentation(_, indent) => (
        state,
        synchronizeIndentationEffect(indent),
      )
    | Actions.ViewSetActiveEditor(_) => (
        state,
        synchronizeEditorEffect(state),
      )
    | Actions.ViewCloseEditor(_) => (state, synchronizeEditorEffect(state))
    | Actions.KeyboardInput(s) => (state, inputEffect(s))
    | Actions.CopyActiveFilepathToClipboard => (
        state,
        copyActiveFilepathToClipboardEffect,
      )

    | Actions.VimDirectoryChanged(directory) =>
      let newState = {
        ...state,
        workspace:
          Some({
            workingDirectory: directory,
            rootName: Filename.basename(directory),
          }),
      };
      (
        newState,
        Isolinear.Effect.batch([
          FileExplorerStore.Effects.load(
            directory,
            state.languageInfo,
            state.iconTheme,
            state.configuration,
          ),
          TitleStoreConnector.Effects.updateTitle(newState),
        ]),
      );

    | _ => (state, Isolinear.Effect.none)
    };
  };

  (updater, stream);
};
