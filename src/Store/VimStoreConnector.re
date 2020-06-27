/*
 * VimStoreConnector.re
 *
 * This module connects vim to the Store:
 * - Translates incoming vim notifications into Actions
 * - Translates Actions into Effects that should run against vim
 */

open EditorCoreTypes;
open Oni_Model;

module Core = Oni_Core;
open Core.Utility;

module Ext = Oni_Extensions;
module Zed_utf8 = Core.ZedBundled;
module CompletionMeet = Feature_LanguageSupport.CompletionMeet;
module Definition = Feature_LanguageSupport.Definition;
module LanguageFeatures = Feature_LanguageSupport.LanguageFeatures;
module Editor = Feature_Editor.Editor;

module Log = (val Core.Log.withNamespace("Oni2.Store.Vim"));

let start =
    (
      ~showUpdateChangelog: bool,
      languageInfo: Ext.LanguageInfo.t,
      getState: unit => State.t,
      getClipboardText,
      setClipboardText,
    ) => {
  let (stream, dispatch) = Isolinear.Stream.create();
  let libvimHasInitialized = ref(false);
  let currentTriggerKey = ref(None);

  Vim.Clipboard.setProvider(reg => {
    let state = getState();
    let yankConfig =
      Selectors.getActiveConfigurationValue(state, c =>
        c.vimUseSystemClipboard
      );

    let isMultipleLines = s => String.contains(s, '\n');

    let splitNewLines = s =>
      s |> StringEx.removeTrailingNewLine |> StringEx.splitNewLines;

    let starReg = Char.code('*');
    let plusReg = Char.code('+');
    let unnamedReg = 0;

    let shouldPullFromClipboard =
      (reg == starReg || reg == plusReg)  // always for '*' and '+'
      || reg == unnamedReg
      && yankConfig.paste; // or if 'paste' set, but unnamed

    if (shouldPullFromClipboard) {
      let clipboardValue = getClipboardText();
      let blockType: Vim.Types.blockType =
        clipboardValue
        |> Option.map(isMultipleLines)
        |> Option.map(
             multiLine => multiLine ? Vim.Types.Line : Vim.Types.Character:
                                                                    bool =>
                                                                    Vim.Types.blockType,
           )
        |> Option.value(~default=Vim.Types.Line: Vim.Types.blockType);

      clipboardValue
      |> Option.map(StringEx.removeTrailingNewLine)
      |> Option.map(StringEx.removeWindowsNewLines)
      |> Option.map(splitNewLines)
      |> Option.map(lines => Vim.Types.{lines, blockType});
    } else {
      None;
    };
  });

  let _: unit => unit =
    Vim.onVersion(() => {
      Actions.OpenFileByPath("oni://Version", None, None) |> dispatch
    });

  let _: unit => unit =
    Vim.Buffer.onLineEndingsChanged((id, lineEndings) => {
      Actions.BufferLineEndingsChanged({id, lineEndings}) |> dispatch
    });

  let handleGoto = gotoType => {
    switch (gotoType) {
    | Vim.Goto.Hover => dispatch(Actions.Hover(Feature_Hover.Command(Show)))

    | Vim.Goto.Definition
    | Vim.Goto.Declaration =>
      Log.debug("Goto definition requested");
      // Get buffer and cursor position
      let state = getState();
      let maybeBuffer = state |> Selectors.getActiveBuffer;

      let editor = Feature_Layout.activeEditor(state.layout);

      let getDefinition = buffer => {
        let id = Core.Buffer.getId(buffer);
        let position = Editor.getPrimaryCursor(editor);
        Definition.getAt(id, position, state.definition)
        |> Option.map((definitionResult: LanguageFeatures.DefinitionResult.t) => {
             Actions.OpenFileByPath(
               definitionResult.uri |> Core.Uri.toFileSystemPath,
               None,
               Some(definitionResult.location),
             )
           });
      };

      Option.map(getDefinition, maybeBuffer)
      |> Option.join
      |> Option.iter(action => dispatch(action));
    };
  };

  let _: unit => unit =
    Vim.onEffect(
      fun
      | Goto(gotoType) => handleGoto(gotoType)
      | TabPage(msg) => dispatch(TabPage(msg))
      | Format(Buffer(_)) =>
        dispatch(
          Actions.Formatting(Feature_Formatting.Command(FormatDocument)),
        )
      | Format(Range(_)) =>
        dispatch(
          Actions.Formatting(Feature_Formatting.Command(FormatRange)),
        ),
    );

  let _: unit => unit =
    Vim.Mode.onChanged(newMode =>
      dispatch(Actions.Vim(Feature_Vim.ModeChanged(newMode)))
    );

  let _: unit => unit =
    Vim.onDirectoryChanged(newDir =>
      dispatch(Actions.VimDirectoryChanged(newDir))
    );

  let _: unit => unit =
    Vim.onMessage((priority, title, message) => {
      dispatch(VimMessageReceived({priority, title, message}))
    });

  let _: unit => unit =
    Vim.onYank(({lines, register, operator, yankType, _}) => {
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
        let text =
          if (Array.length(lines) == 1 && yankType == Line) {
            lines[0] ++ "\n";
          } else {
            String.concat("\n", Array.to_list(lines));
          };

        setClipboardText(text);
      };
    });

  let _: unit => unit =
    Vim.onWriteFailure((_reason, _buffer) => dispatch(WriteFailure));

  let _: unit => unit =
    Vim.Buffer.onFilenameChanged(meta => {
      Log.debugf(m => m("Buffer metadata changed: %n", meta.id));
      let fileType =
        switch (meta.filePath) {
        | Some(v) =>
          Some(Ext.LanguageInfo.getLanguageFromFilePath(languageInfo, v))
        | None => None
        };

      dispatch(
        Actions.BufferFilenameChanged({
          id: meta.id,
          newFileType: fileType,
          newFilePath: meta.filePath,
          isModified: meta.modified,
          version: meta.version,
        }),
      );
    });

  let _: unit => unit =
    Vim.Buffer.onModifiedChanged((id, isModified) => {
      Log.debugf(m => m("Buffer metadata changed: %n | %b", id, isModified));
      dispatch(Actions.BufferSetModified(id, isModified));
    });

  let _: unit => unit =
    Vim.Buffer.onWrite(id => {dispatch(Actions.BufferSaved(id))});

  let _: unit => unit =
    Vim.Search.onStopSearchHighlight(() => {
      let buffer = Vim.Buffer.getCurrent();
      let id = Vim.Buffer.getId(buffer);
      dispatch(Actions.SearchClearHighlights(id));
    });

  let _: unit => unit =
    Vim.onQuit((quitType, force) =>
      switch (quitType) {
      | QuitAll => dispatch(Quit(force))
      | QuitOne(buf) => dispatch(QuitBuffer(buf, force))
      }
    );

  let _: unit => unit =
    Vim.onTerminal(({cmd, curwin, closeOnFinish, _}) => {
      let splitDirection =
        if (curwin) {Feature_Terminal.Current} else {
          Feature_Terminal.Horizontal
        };

      dispatch(
        Actions.Terminal(
          Command(
            NewTerminal({cmd, splitDirection, closeOnExit: closeOnFinish}),
          ),
        ),
      );
    });

  let _: unit => unit =
    Vim.Visual.onRangeChanged(vr => {
      open Vim.VisualRange;

      let {visualType, range} = vr;
      let vr =
        Core.VisualRange.create(
          ~mode=visualType,
          Range.{
            start: {
              ...range.start,
              column: range.start.column,
            },
            stop: {
              ...range.stop,
              column: range.stop.column,
            },
          },
        );

      let editorId =
        Feature_Layout.activeEditor(getState().layout) |> Editor.getId;
      dispatch(Editor({editorId, msg: SelectionChanged(vr)}));
    });

  let _: unit => unit =
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

      Log.trace("Vim.Window.onSplit: " ++ buf);

      let command =
        switch (splitType) {
        | Vim.Types.Vertical =>
          Actions.OpenFileByPath(buf, Some(`Vertical), None)
        | Vim.Types.Horizontal =>
          Actions.OpenFileByPath(buf, Some(`Horizontal), None)
        | Vim.Types.TabPage => Actions.OpenFileInNewLayout(buf)
        };
      dispatch(command);
    });

  let _: unit => unit =
    Vim.Buffer.onEnter(buf => {
      let metadata = Vim.BufferMetadata.ofBuffer(buf);

      if (metadata.id == 1 && ! libvimHasInitialized^) {
        Log.info("Ignoring initial buffer");
      } else {
        let fileType =
          switch (metadata.filePath) {
          | Some(v) =>
            Some(Ext.LanguageInfo.getLanguageFromFilePath(languageInfo, v))
          | None => None
          };

        let lineEndings: option(Vim.lineEnding) =
          Vim.Buffer.getLineEndings(buf);

        let state = getState();

        let buffer =
          (
            switch (Selectors.getBufferById(state, metadata.id)) {
            | Some(buf) => buf
            | None =>
              Oni_Core.Buffer.ofMetadata(
                ~id=metadata.id,
                ~version=- metadata.version,
                ~filePath=metadata.filePath,
                ~modified=metadata.modified,
              )
            }
          )
          |> Oni_Core.Buffer.setFileType(fileType);

        dispatch(
          Actions.BufferEnter({
            id: metadata.id,
            buffer,
            fileType,
            lineEndings,
            // Version must be 0 so that a buffer update will be processed
            version: 0,
            isModified: metadata.modified,
            filePath: metadata.filePath,
          }),
        );
      };
    });

  let _: unit => unit =
    Vim.Buffer.onUpdate(update => {
      open Vim.BufferUpdate;
      Log.debugf(m => m("Buffer update: %n", update.id));
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
          ~startLine=Index.fromOneBased(update.startLine),
          ~endLine=Index.fromOneBased(endLine),
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
        maybeBuffer
        |> Option.iter(oldBuffer => {
             let newBuffer = Core.Buffer.update(oldBuffer, bu);
             dispatch(
               Actions.BufferUpdate({
                 update: bu,
                 newBuffer,
                 oldBuffer,
                 triggerKey: currentTriggerKey^,
               }),
             );
           });
      } else {
        Log.debugf(m => m("Skipped buffer update at: %i", update.version));
      };
    });

  let _: unit => unit =
    Vim.CommandLine.onEnter(c =>
      dispatch(Actions.QuickmenuShow(Wildmenu(c.cmdType)))
    );

  let lastCompletionMeet = ref(None);
  let isCompleting = ref(false);

  let checkCommandLineCompletions = () => {
    Log.debug("checkCommandLineCompletions");

    let completions = Vim.CommandLine.getCompletions();

    Log.debugf(m => m("  got %n completions.", Array.length(completions)));

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

  let _: unit => unit =
    Vim.CommandLine.onUpdate(({text, position: cursorPosition, _}) => {
      dispatch(Actions.QuickmenuCommandlineUpdated(text, cursorPosition));

      let cmdlineType = Vim.CommandLine.getType();
      switch (cmdlineType) {
      | Ex =>
        let text =
          switch (Vim.CommandLine.getText()) {
          | Some(v) => v
          | None => ""
          };
        let meet = Feature_Vim.CommandLine.getCompletionMeet(text);
        lastCompletionMeet := meet;

        isCompleting^ ? () : checkCommandLineCompletions();

      | SearchForward
      | SearchReverse =>
        let highlights = Vim.Search.getHighlights();

        let sameLineFilter = (range: Range.t) =>
          range.start.line == range.stop.line;

        let buffer = Vim.Buffer.getCurrent();
        let id = Vim.Buffer.getId(buffer);

        let highlightList =
          highlights |> Array.to_list |> List.filter(sameLineFilter);
        dispatch(SearchSetHighlights(id, highlightList));

      | _ => ()
      };
    });

  let _: unit => unit =
    Vim.CommandLine.onLeave(() => {
      lastCompletionMeet := None;
      isCompleting := false;
      dispatch(Actions.QuickmenuClose);
    });

  let initEffect =
    Isolinear.Effect.create(~name="vim.init", () => {
      if (showUpdateChangelog
          && Core.BuildInfo.commitId != Persistence.Global.version()) {
        dispatch(
          Actions.OpenFileByPath(Core.BufferPath.updateChangelog, None, None),
        );
      };
      libvimHasInitialized := true;
    });

  let updateActiveEditorCursors = cursors => {
    let editorId =
      Feature_Layout.activeEditor(getState().layout) |> Editor.getId;

    dispatch(Actions.Editor({editorId, msg: CursorsChanged(cursors)}));
  };

  let isVimKey = key => {
    !String.equal(key, "<S-SHIFT>")
    && !String.equal(key, "<A-SHIFT>")
    && !String.equal(key, "<D-SHIFT>")
    && !String.equal(key, "<D->")
    && !String.equal(key, "<D-A->")
    && !String.equal(key, "<D-S->")
    && !String.equal(key, "<C->")
    && !String.equal(key, "<A-C->")
    && !String.equal(key, "<SHIFT>")
    && !String.equal(key, "<S-C->");
  };

  let commandEffect = cmd => {
    Isolinear.Effect.create(~name="vim.command", () => {
      // TODO: Hook up effect handler
      ignore(Vim.command(cmd): Vim.Context.t)
    });
  };

  let inputEffect = key =>
    Isolinear.Effect.createWithDispatch(~name="vim.input", dispatch =>
      if (isVimKey(key)) {
        // Set cursors based on current editor
        let state = getState();
        let editorId =
          Feature_Layout.activeEditor(state.layout) |> Editor.getId;

        let context = Oni_Model.VimContext.current(state);

        currentTriggerKey := Some(key);
        let {cursors, topLine: newTopLine, leftColumn: newLeftColumn, _}: Vim.Context.t =
          Vim.input(~context, key);
        currentTriggerKey := None;

        dispatch(Actions.Editor({editorId, msg: CursorsChanged(cursors)}));
        dispatch(
          Actions.Editor({editorId, msg: ScrollToLine(newTopLine - 1)}),
        );
        dispatch(
          Actions.Editor({editorId, msg: ScrollToColumn(newLeftColumn)}),
        );

        Log.debug("handled key: " ++ key);
      }
    );

  let openBufferEffect = (~onComplete, filePath) =>
    Isolinear.Effect.create(~name="vim.openBuffer", () => {
      let buffer = Vim.Buffer.openFile(filePath);
      let bufferId = Vim.Buffer.getId(buffer);

      dispatch(onComplete(bufferId));
    });

  let gotoLocationEffect = (editorId, location: Location.t) =>
    Isolinear.Effect.create(~name="vim.gotoLocation", () => {
      let cursor = (location :> Vim.Cursor.t);
      updateActiveEditorCursors([cursor]);

      let topLine: int = max(Index.toZeroBased(location.line) - 10, 0);

      dispatch(Actions.Editor({editorId, msg: ScrollToLine(topLine)}));
    });

  let addBufferRendererEffect = (bufferId, renderer) =>
    Isolinear.Effect.create(~name="vim.addBufferRenderer", () => {
      dispatch(
        Actions.BufferRenderer(RendererAvailable(bufferId, renderer)),
      )
    });

  let openTutorEffect =
    Isolinear.Effect.create(~name="vim.tutor", () => {
      let filename = Filename.temp_file("tutor", "");

      let input = open_in(Revery.Environment.getAssetPath("tutor"));
      let content = really_input_string(input, in_channel_length(input));
      close_in(input);

      let output = open_out(filename);
      output_string(output, content);
      close_out(output);

      ignore(Vim.Buffer.openFile(filename): Vim.Buffer.t);
    });

  let applyCompletionEffect = completion =>
    Isolinear.Effect.create(~name="vim.applyCommandlineCompletion", () =>
      switch (lastCompletionMeet^) {
      | None => ()
      | Some(position) =>
        isCompleting := true;
        let currentPos = ref(Vim.CommandLine.getPosition());
        while (currentPos^ > position) {
          let _: Vim.Context.t = Vim.input("<bs>");
          currentPos := Vim.CommandLine.getPosition();
        };

        let completion = Path.trimTrailingSeparator(completion);
        let latestContext: Vim.Context.t = Core.VimEx.inputString(completion);
        updateActiveEditorCursors(latestContext.cursors);
        isCompleting := false;
      }
    );

  let pasteIntoEditorAction =
    Isolinear.Effect.create(~name="vim.clipboardPaste", () => {
      let isCmdLineMode = Vim.Mode.getCurrent() == Vim.Types.CommandLine;
      let isInsertMode = Vim.Mode.getCurrent() == Vim.Types.Insert;

      if (isInsertMode || isCmdLineMode) {
        getClipboardText()
        |> Option.iter(text => {
             if (!isCmdLineMode) {
               Vim.command("set paste") |> ignore;
             };

             let latestContext: Vim.Context.t =
               Oni_Core.VimEx.inputString(text);

             if (!isCmdLineMode) {
               updateActiveEditorCursors(latestContext.cursors);
               Vim.command("set nopaste") |> ignore;
             };
           });
      };
    });

  let copyActiveFilepathToClipboardEffect =
    Isolinear.Effect.create(~name="vim.copyActiveFilepathToClipboard", () =>
      switch (Vim.Buffer.getCurrent() |> Vim.Buffer.getFilename) {
      | Some(filename) => setClipboardText(filename)
      | None => ()
      }
    );

  let prevViml = ref([]);
  let synchronizeViml = configuration =>
    Isolinear.Effect.create(~name="vim.synchronizeViml", () => {
      let lines =
        Core.Configuration.getValue(c => c.experimentalVimL, configuration);

      if (prevViml^ !== lines) {
        List.iter(
          l => {
            Log.info("Running VimL from config: " ++ l);
            Vim.command(l) |> ignore;
            Log.info("VimL command completed.");
          },
          lines,
        );
        prevViml := lines;
      };
    });

  let undoEffect =
    Isolinear.Effect.create(~name="vim.undo", () => {
      let _: Vim.Context.t = Vim.input("<esc>");
      let _: Vim.Context.t = Vim.input("<esc>");
      let {cursors, _}: Vim.Context.t = Vim.input("u");
      updateActiveEditorCursors(cursors);
      ();
    });

  let redoEffect =
    Isolinear.Effect.create(~name="vim.redo", () => {
      let _: Vim.Context.t = Vim.input("<esc>");
      let _: Vim.Context.t = Vim.input("<esc>");
      let {cursors, _}: Vim.Context.t = Vim.input("<c-r>");
      updateActiveEditorCursors(cursors);
      ();
    });

  let saveEffect =
    Isolinear.Effect.create(~name="vim.save", () => {
      let _: Vim.Context.t = Vim.input("<esc>");
      let _: Vim.Context.t = Vim.input("<esc>");
      let _: Vim.Context.t = Vim.input(":");
      let _: Vim.Context.t = Vim.input("w");
      let _: Vim.Context.t = Vim.input("<CR>");
      ();
    });

  let escapeEffect =
    Isolinear.Effect.create(~name="vim.esc", () => {
      let _: Vim.Context.t = Vim.input("<esc>");
      ();
    });

  let indentEffect =
    Isolinear.Effect.create(~name="vim.indent", () => {
      let _: Vim.Context.t = Vim.input(">");
      let _: Vim.Context.t = Vim.input("g");
      let _: Vim.Context.t = Vim.input("v");
      ();
    });

  let outdentEffect =
    Isolinear.Effect.create(~name="vim.outdent", () => {
      let _: Vim.Context.t = Vim.input("<");
      let _: Vim.Context.t = Vim.input("g");
      let _: Vim.Context.t = Vim.input("v");
      ();
    });

  let setTerminalLinesEffect = (~editorId, ~bufferId, lines: array(string)) => {
    Isolinear.Effect.create(~name="vim.setTerminalLinesEffect", () => {
      let () =
        bufferId
        |> Vim.Buffer.getById
        |> Option.iter(buf => {
             Vim.Buffer.setModifiable(~modifiable=true, buf);
             Vim.Buffer.setLines(~lines, buf);
             Vim.Buffer.setModifiable(~modifiable=false, buf);
             Vim.Buffer.setReadOnly(~readOnly=true, buf);
           });

      // Clear out previous mode
      let _: Vim.Context.t = Vim.input("<esc>");
      let _: Vim.Context.t = Vim.input("<esc>");
      // Jump to bottom
      let _: Vim.Context.t = Vim.input("g");
      let _: Vim.Context.t = Vim.input("g");
      let _: Vim.Context.t = Vim.input("G");
      let {cursors, topLine: newTopLine, leftColumn: newLeftColumn, _}: Vim.Context.t =
        Vim.input("$");

      // Update the editor, which is the source of truth for cursor position
      dispatch(Actions.Editor({editorId, msg: CursorsChanged(cursors)}));
      dispatch(
        Actions.Editor({editorId, msg: ScrollToLine(newTopLine - 1)}),
      );
      dispatch(
        Actions.Editor({editorId, msg: ScrollToColumn(newLeftColumn)}),
      );
    });
  };

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | ConfigurationSet(configuration) => (
        state,
        synchronizeViml(configuration),
      )

    | Command("editor.action.clipboardPasteAction") => (
        state,
        pasteIntoEditorAction,
      )
    | Command("undo") => (state, undoEffect)
    | Command("redo") => (state, redoEffect)
    | Command("workbench.action.files.save") => (state, saveEffect)
    | Command("indent") => (state, indentEffect)
    | Command("outdent") => (state, outdentEffect)
    | Command("editor.action.indentLines") => (state, indentEffect)
    | Command("editor.action.outdentLines") => (state, outdentEffect)
    | Command("vim.esc") => (state, escapeEffect)
    | Command("vim.tutor") => (state, openTutorEffect)
    | VimExecuteCommand(cmd) => (state, commandEffect(cmd))

    | ListFocusUp
    | ListFocusDown
    | ListFocus(_) =>
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

    | Init => (state, initEffect)

    | OpenFileByPath(path, maybeDirection, maybeLocation) =>
      /* If a split was requested, create that first! */
      let state' =
        switch (maybeDirection) {
        | None => state
        | Some(direction) => {
            ...state,
            layout: Feature_Layout.split(direction, state.layout),
          }
        };
      (
        state',
        openBufferEffect(
          ~onComplete=bufferId => BufferOpened(path, maybeLocation, bufferId),
          path,
        ),
      );
    | BufferOpened(path, maybeLocation, bufferId) =>
      let maybeRenderer =
        switch (Core.BufferPath.parse(path)) {
        | Terminal({bufferId, _}) =>
          Some(
            BufferRenderer.Terminal({
              title: "Terminal",
              id: bufferId,
              insertMode: true,
            }),
          )
        | Version => Some(BufferRenderer.Version)
        | UpdateChangelog =>
          Some(
            BufferRenderer.UpdateChangelog({
              since: Persistence.Global.version(),
            }),
          )
        | Welcome => Some(BufferRenderer.Welcome)
        | Changelog => Some(BufferRenderer.FullChangelog)
        | FilePath(_) => None
        };

      let editorId =
        Feature_Layout.activeEditor(state.layout) |> Editor.getId;

      (
        state,
        Isolinear.Effect.batch([
          maybeRenderer
          |> Option.map(addBufferRendererEffect(bufferId))
          |> Option.value(~default=Isolinear.Effect.none),
          maybeLocation
          |> Option.map(gotoLocationEffect(editorId))
          |> Option.value(~default=Isolinear.Effect.none),
        ]),
      );

    | OpenFileInNewLayout(path) =>
      let state = {
        ...state,
        layout: Feature_Layout.addLayoutTab(state.layout),
      };
      (
        state,
        openBufferEffect(
          ~onComplete=bufferId => BufferOpenedForLayout(bufferId),
          path,
        ),
      );
    | BufferOpenedForLayout(_bufferId) => (state, Isolinear.Effect.none)

    | Terminal(Command(NormalMode)) =>
      let maybeBufferId =
        state
        |> Selectors.getActiveBuffer
        |> Option.map(Oni_Core.Buffer.getId);

      let maybeTerminalId =
        maybeBufferId
        |> Option.map(id =>
             BufferRenderers.getById(id, state.bufferRenderers)
           )
        |> OptionEx.flatMap(
             fun
             | BufferRenderer.Terminal({id, _}) => Some(id)
             | _ => None,
           );

      let editorId =
        Feature_Layout.activeEditor(state.layout) |> Editor.getId;

      let (state, effect) =
        OptionEx.map2(
          (bufferId, terminalId) => {
            let colorTheme = Feature_Theme.colors(state.colorTheme);
            let (lines, highlights) =
              Feature_Terminal.getLinesAndHighlights(
                ~colorTheme,
                ~terminalId,
              );
            let syntaxHighlights =
              List.fold_left(
                (acc, curr) => {
                  let (line, tokens) = curr;
                  Feature_Syntax.setTokensForLine(
                    ~bufferId,
                    ~line,
                    ~tokens,
                    acc,
                  );
                },
                state.syntaxHighlights,
                highlights,
              );

            let syntaxHighlights =
              syntaxHighlights |> Feature_Syntax.ignore(~bufferId);

            let layout =
              Feature_Layout.map(
                editor =>
                  Editor.getBufferId(editor) == bufferId
                    ? Editor.setFont(~font=state.terminalFont, editor)
                    : editor,
                state.layout,
              );

            (
              {...state, layout, syntaxHighlights},
              setTerminalLinesEffect(~bufferId, ~editorId, lines),
            );
          },
          maybeBufferId,
          maybeTerminalId,
        )
        |> Option.value(~default=(state, Isolinear.Effect.none));

      (state, effect);

    | KeyboardInput(s) => (state, inputEffect(s))

    | CopyActiveFilepathToClipboard => (
        state,
        copyActiveFilepathToClipboardEffect,
      )

    | VimDirectoryChanged(workingDirectory) =>
      let newState = {
        ...state,
        workspace: {
          workingDirectory,
          rootName: Filename.basename(workingDirectory),
        },
      };
      (
        newState,
        Isolinear.Effect.batch([
          FileExplorerStore.Effects.load(
            workingDirectory,
            state.languageInfo,
            state.iconTheme,
            state.configuration,
            ~onComplete=tree =>
            Actions.FileExplorer(TreeLoaded(tree))
          ),
          TitleStoreConnector.Effects.updateTitle(newState),
        ]),
      );

    | VimMessageReceived({priority, message, _}) =>
      let kind =
        switch (priority) {
        | Error => Feature_Notification.Error
        | Warning => Feature_Notification.Warning
        | Info => Feature_Notification.Info
        };

      (
        state,
        Feature_Notification.Effects.create(~kind, message)
        |> Isolinear.Effect.map(msg => Actions.Notification(msg)),
      );

    | WriteFailure => (
        {...state, modal: Some(Feature_Modals.writeFailure)},
        Isolinear.Effect.none,
      )

    | FileChanged(event) =>
      switch (Selectors.getActiveBuffer(state)) {
      | Some(buffer)
          when
            Core.Buffer.getFilePath(buffer) == Some(event.path)
            && !Core.Buffer.isModified(buffer) => (
          state,
          Service_Vim.reload(),
        )
      | _ => (state, Isolinear.Effect.none)
      }

    | TabPage(Goto(index)) => (
        {
          ...state,
          layout: Feature_Layout.gotoLayoutTab(index - 1, state.layout),
        },
        Isolinear.Effect.none,
      )

    | TabPage(GotoRelative(delta)) when delta < 0 => (
        {
          ...state,
          layout:
            Feature_Layout.previousLayoutTab(~count=- delta, state.layout),
        },
        Isolinear.Effect.none,
      )

    | TabPage(GotoRelative(delta)) => (
        {
          ...state,
          layout: Feature_Layout.nextLayoutTab(~count=delta, state.layout),
        },
        Isolinear.Effect.none,
      )

    | TabPage(Move(index)) => (
        {
          ...state,
          layout:
            Feature_Layout.moveActiveLayoutTabTo(index - 1, state.layout),
        },
        Isolinear.Effect.none,
      )

    | TabPage(MoveRelative(delta)) => (
        {
          ...state,
          layout:
            Feature_Layout.moveActiveLayoutTabRelative(delta, state.layout),
        },
        Isolinear.Effect.none,
      )

    | TabPage(Close(0)) =>
      switch (Feature_Layout.removeActiveLayoutTab(state.layout)) {
      | Some(layout) => ({...state, layout}, Isolinear.Effect.none)
      | None => (state, Isolinear.Effect.none)
      }

    | TabPage(Close(index)) =>
      switch (Feature_Layout.removeLayoutTab(index - 1, state.layout)) {
      | Some(layout) => ({...state, layout}, Isolinear.Effect.none)
      | None => (state, Isolinear.Effect.none)
      }

    | TabPage(Only(0)) => (
        {
          ...state,
          layout: state.layout |> Feature_Layout.removeOtherLayoutTabs,
        },
        Isolinear.Effect.none,
      )

    | TabPage(Only(index)) => (
        {
          ...state,
          layout:
            state.layout
            |> Feature_Layout.gotoLayoutTab(index - 1)
            |> Feature_Layout.removeOtherLayoutTabs,
        },
        Isolinear.Effect.none,
      )

    | _ => (state, Isolinear.Effect.none)
    };
  };

  (updater, stream);
};

let subscriptions = (state: State.t) => {
  state.buffers
  |> Core.IntMap.bindings
  |> List.filter_map(((_key, buffer)) =>
       buffer
       |> Core.Buffer.getFilePath
       |> Option.map(path =>
            Service_FileWatcher.watch(~path, ~onEvent=event =>
              Actions.FileChanged(event)
            )
          )
     );
};
