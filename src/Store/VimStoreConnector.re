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

module Zed_utf8 = Core.ZedBundled;
module LanguageFeatures = Feature_LanguageSupport.LanguageFeatures;
module Editor = Feature_Editor.Editor;

module Log = (val Core.Log.withNamespace("Oni2.Store.Vim"));

let start =
    (
      languageInfo: Exthost.LanguageInfo.t,
      getState: unit => State.t,
      getClipboardText,
      setClipboardText,
    ) => {
  let (stream, dispatch) = Isolinear.Stream.create();
  let libvimHasInitialized = ref(false);
  let currentTriggerKey = ref(None);

  let colorSchemeProvider = pattern => {
    getState().extensions
    |> Feature_Extensions.themesByName(~filter=pattern)
    |> Array.of_list;
  };

  Vim.Clipboard.setProvider(reg => {
    let state = getState();
    let yankConfig =
      Selectors.getActiveConfigurationValue(state, c =>
        c.vimUseSystemClipboard
      );

    let starReg = Char.code('*');
    let plusReg = Char.code('+');
    let unnamedReg = 0;

    let shouldPullFromClipboard =
      (reg == starReg || reg == plusReg)  // always for '*' and '+'
      || reg == unnamedReg
      && yankConfig.paste; // or if 'paste' set, but unnamed

    if (shouldPullFromClipboard) {
      let maybeClipboardValue = getClipboardText();
      maybeClipboardValue
      |> Option.map(clipboardValue => {
           let (multiLine, lines) = StringEx.splitLines(clipboardValue);
           let blockType: Vim.Types.blockType =
             multiLine ? Vim.Types.Line : Vim.Types.Character;
           Vim.Types.{lines, blockType};
         });
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
      Actions.Buffers(Feature_Buffers.LineEndingsChanged({id, lineEndings}))
      |> dispatch
    });

  let _: unit => unit =
    Vim.Buffer.onFiletypeChanged(metadata => {
      let Vim.BufferMetadata.{id, fileType, _} = metadata;

      switch (fileType) {
      | None => ()
      | Some(fileType) =>
        Log.infof(m => m("Setting filetype %s for buffer %d", fileType, id));
        dispatch(
          Actions.Buffers(
            Feature_Buffers.FileTypeChanged({
              id,
              fileType: Oni_Core.Buffer.FileType.explicit(fileType),
            }),
          ),
        );
      };
    });

  let handleGoto = gotoType => {
    switch (gotoType) {
    | Vim.Goto.Hover =>
      dispatch(
        Actions.LanguageSupport(Feature_LanguageSupport.Msg.Hover.show),
      )

    | Vim.Goto.Definition
    | Vim.Goto.Declaration =>
      Log.info("Goto definition requested");
      // Get buffer and cursor position
      let state = getState();
      let maybeBuffer = state |> Selectors.getActiveBuffer;

      let getDefinition = buffer => {
        let id = Core.Buffer.getId(buffer);
        Feature_LanguageSupport.Definition.get(
          ~bufferId=id,
          state.languageSupport,
        )
        |> Option.map((definitionResult: Exthost.DefinitionLink.t) => {
             let {startLineNumber, startColumn, _}: Exthost.OneBasedRange.t =
               definitionResult.range;

             let position =
               CharacterPosition.{
                 line: EditorCoreTypes.LineNumber.ofOneBased(startLineNumber),
                 character: CharacterIndex.ofInt(startColumn - 1),
               };

             Actions.OpenFileByPath(
               definitionResult.uri |> Core.Uri.toFileSystemPath,
               None,
               Some(position),
             );
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
          Actions.LanguageSupport(
            Feature_LanguageSupport.Msg.Formatting.formatDocument,
          ),
        )
      | Format(Range({startLine, endLine, _})) =>
        dispatch(
          Actions.LanguageSupport(
            Feature_LanguageSupport.Msg.Formatting.formatRange(
              ~startLine,
              ~endLine,
            ),
          ),
        )
      | ModeChanged(newMode) => {
          dispatch(Actions.Vim(Feature_Vim.ModeChanged(newMode)));

          let editorId =
            Feature_Layout.activeEditor(getState().layout) |> Editor.getId;

          switch (newMode) {
          | Visual({range})
          | Select({range}) =>
            dispatch(
              Editor({
                scope: Oni_Model.EditorScope.Editor(editorId),
                msg: SelectionChanged(Oni_Core.VisualRange.ofVim(range)),
              }),
            )
          | _ =>
            dispatch(
              Editor({
                scope: Oni_Model.EditorScope.Editor(editorId),
                msg: SelectionCleared,
              }),
            )
          };
        }
      | SettingChanged(setting) =>
        dispatch(Actions.Vim(Feature_Vim.SettingChanged(setting)))

      | ColorSchemeChanged(maybeColorScheme) =>
        switch (maybeColorScheme) {
        | None => dispatch(Actions.Theme(Feature_Theme.Msg.openThemePicker))
        | Some(colorScheme) =>
          dispatch(Actions.ThemeLoadByName(colorScheme))
        }

      | MacroRecordingStarted({register}) =>
        dispatch(
          Actions.Vim(
            Feature_Vim.MacroRecordingStarted({register: register}),
          ),
        )

      | MacroRecordingStopped(_) =>
        dispatch(Actions.Vim(Feature_Vim.MacroRecordingStopped)),
    );

  let _: unit => unit =
    Vim.onDirectoryChanged(newDir =>
      dispatch(Actions.DirectoryChanged(newDir))
    );

  let _: unit => unit =
    Vim.onMessage((priority, title, message) => {
      dispatch(VimMessageReceived({priority, title, message}))
    });

  let _: unit => unit =
    Vim.onYank(
      (
        {
          lines,
          register,
          operator,
          yankType,
          startLine,
          startColumn,
          endLine,
          endColumn,
        },
      ) => {
      let state = getState();

      if (operator == Vim.Yank.Yank) {
        let visualType =
          switch (yankType) {
          | Block => Vim.Types.Block
          | Line => Vim.Types.Line
          | Char => Vim.Types.Character
          };

        let range =
          ByteRange.{
            start:
              BytePosition.{
                line: LineNumber.ofOneBased(startLine),
                byte: ByteIndex.ofInt(startColumn),
              },
            stop:
              BytePosition.{
                line: LineNumber.ofOneBased(endLine),
                byte: ByteIndex.ofInt(endColumn),
              },
          };
        let visualRange =
          Oni_Core.VisualRange.create(~mode=visualType, range);
        dispatch(Actions.Yank({range: visualRange}));
      };

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
      // TODO: This isn't buffer aware, so it won't be able to deal with the
      // firstline way of getting syntax, which means if that is in use,
      // it will get wiped when renaming the file.
      //
      // Other notes: The file path is going to be updated in BufferFilenameChanged,
      // so you can't use the buffer here, as it will have the old path.
      // Additionally, the syntax server will need to be notified on the filetype
      // change / hook it up to onFileTypeChanged.
      let fileType =
        switch (meta.filePath) {
        | Some(v) =>
          Exthost.LanguageInfo.getLanguageFromFilePath(languageInfo, v)
          |> Oni_Core.Buffer.FileType.inferred
        | None => Oni_Core.Buffer.FileType.none
        };

      dispatch(
        Actions.Buffers(
          Feature_Buffers.FilenameChanged({
            id: meta.id,
            newFileType: fileType,
            newFilePath: meta.filePath,
            isModified: meta.modified,
            version: meta.version,
          }),
        ),
      );
    });

  let _: unit => unit =
    Vim.Buffer.onModifiedChanged((id, isModified) => {
      Log.debugf(m => m("Buffer metadata changed: %n | %b", id, isModified));
      dispatch(
        Actions.Buffers(Feature_Buffers.ModifiedSet(id, isModified)),
      );
    });

  let _: unit => unit =
    Vim.Buffer.onWrite(id => {
      dispatch(Actions.Buffers(Feature_Buffers.Saved(id)))
    });

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
            Exthost.LanguageInfo.getLanguageFromFilePath(languageInfo, v)
            |> Oni_Core.Buffer.FileType.inferred
          | None => Oni_Core.Buffer.FileType.none
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
                ~font=state.editorFont,
                ~version=- metadata.version,
                ~filePath=metadata.filePath,
                ~modified=metadata.modified,
              )
            }
          )
          |> Oni_Core.Buffer.setFileType(fileType);

        dispatch(
          Actions.Buffers(
            Feature_Buffers.Entered({
              id: metadata.id,
              buffer,
              fileType,
              lineEndings,
              // Version must be 0 so that a buffer update will be processed
              version: 0,
              isModified: metadata.modified,
              filePath: metadata.filePath,
              font: Oni_Core.Buffer.getFont(buffer),
            }),
          ),
        );
      };
    });

  let _: unit => unit =
    Vim.Buffer.onUpdate(update => {
      open Vim.BufferUpdate;
      Log.debugf(m => m("Buffer update: %n", update.id));
      open State;

      let isFull = update.endLine == (-1);

      let maybeBuffer = Feature_Buffers.get(update.id, getState().buffers);

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
          ~startLine=EditorCoreTypes.LineNumber.ofOneBased(update.startLine),
          ~endLine=EditorCoreTypes.LineNumber.ofOneBased(endLine),
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
             // If the first line changes, re-run the file detection.
             let firstLineChanged =
               EditorCoreTypes.(
                 LineNumber.equals(bu.startLine, LineNumber.zero)
               )
               || bu.isFull;

             let newBuffer =
               if (firstLineChanged) {
                 let fileType =
                   Oni_Core.Buffer.FileType.inferred(
                     Exthost.LanguageInfo.getLanguageFromBuffer(
                       languageInfo,
                       newBuffer,
                     ),
                   );
                 newBuffer |> Core.Buffer.setFileType(fileType);
               } else {
                 newBuffer;
               };

             dispatch(
               Actions.Buffers(
                 Feature_Buffers.Update({
                   update: bu,
                   newBuffer,
                   oldBuffer,
                   triggerKey: currentTriggerKey^,
                 }),
               ),
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

    let position = Vim.CommandLine.getPosition();
    Vim.CommandLine.getText()
    |> Option.iter(commandStr =>
         if (position == String.length(commandStr)) {
           let completions =
             Vim.CommandLine.getCompletions(~colorSchemeProvider, ());

           Log.debugf(m =>
             m("  got %n completions.", Array.length(completions))
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
                   handle: None,
                 },
               completions,
             );

           dispatch(Actions.QuickmenuUpdateFilterProgress(items, Complete));
         }
       );
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

        let sameLineFilter = (range: ByteRange.t) =>
          EditorCoreTypes.LineNumber.(range.start.line == range.stop.line);

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
      libvimHasInitialized := true
    });

  let updateActiveEditorCursors = cursors => {
    let editorId =
      Feature_Layout.activeEditor(getState().layout) |> Editor.getId;

    dispatch(
      Actions.Editor({
        scope: EditorScope.Editor(editorId),
        msg: CursorsChanged(cursors),
      }),
    );
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

  let inputEffect = (~isText, key) =>
    Isolinear.Effect.createWithDispatch(~name="vim.input", dispatch =>
      if (isVimKey(key)) {
        // Set cursors based on current editor
        let state = getState();
        let editorId =
          Feature_Layout.activeEditor(state.layout) |> Editor.getId;

        let context = Oni_Model.VimContext.current(state);

        currentTriggerKey := Some(key);
        let {cursors, topLine: newTopLine, leftColumn: newLeftColumn, _}: Vim.Context.t =
          isText ? Vim.input(~context, key) : Vim.key(~context, key);
        currentTriggerKey := None;

        // TODO: This has a sensitive timing dependency - the scroll actions need to happen first,
        // and then the cursor changed. This is because the cursor changed may impact the scroll
        // (ensuring the cursor is visible).
        //
        // Ultimately - we want to get rid of those topline/columnline sync, and have Onivim wholly
        // own the 'scroll' experience.
        // ie: https://github.com/onivim/oni2/pull/2067/commits/a1eb60dd3b9679d0aabda83616c4400ebe1eb3d3
        dispatch(
          Actions.Editor({
            scope: EditorScope.Editor(editorId),
            msg: ScrollToLine(newTopLine - 1),
          }),
        );
        dispatch(
          Actions.Editor({
            scope: EditorScope.Editor(editorId),
            msg: ScrollToColumn(newLeftColumn),
          }),
        );

        dispatch(
          Actions.Editor({
            scope: EditorScope.Editor(editorId),
            msg: CursorsChanged(cursors),
          }),
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

  let gotoLocationEffect = (editorId, location: BytePosition.t) =>
    Isolinear.Effect.create(~name="vim.gotoLocation", () => {
      updateActiveEditorCursors([location]);

      let topLine: int = max(LineNumber.toZeroBased(location.line) - 10, 0);

      dispatch(
        Actions.Editor({
          scope: EditorScope.Editor(editorId),
          msg: ScrollToLine(topLine),
        }),
      );
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
          let _: Vim.Context.t = Vim.key("<bs>");
          currentPos := Vim.CommandLine.getPosition();
        };

        let completion = Path.trimTrailingSeparator(completion);
        let latestContext: Vim.Context.t = Core.VimEx.inputString(completion);
        updateActiveEditorCursors(latestContext.cursors);
        isCompleting := false;
      }
    );

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
      let _: Vim.Context.t = Vim.key("<esc>");
      let _: Vim.Context.t = Vim.key("<esc>");
      let {cursors, _}: Vim.Context.t = Vim.key("u");
      updateActiveEditorCursors(cursors);
      ();
    });

  let redoEffect =
    Isolinear.Effect.create(~name="vim.redo", () => {
      let _: Vim.Context.t = Vim.key("<esc>");
      let _: Vim.Context.t = Vim.key("<esc>");
      let {cursors, _}: Vim.Context.t = Vim.key("<c-r>");
      updateActiveEditorCursors(cursors);
      ();
    });

  let saveEffect =
    Isolinear.Effect.create(~name="vim.save", () => {
      let _: Vim.Context.t = Vim.key("<esc>");
      let _: Vim.Context.t = Vim.key("<esc>");
      let _: Vim.Context.t = Vim.key(":");
      let _: Vim.Context.t = Vim.key("w");
      let _: Vim.Context.t = Vim.key("<CR>");
      ();
    });

  let escapeEffect =
    Isolinear.Effect.create(~name="vim.esc", () => {
      let _: Vim.Context.t = Vim.key("<esc>");
      ();
    });

  let indentEffect =
    Isolinear.Effect.create(~name="vim.indent", () => {
      let _: Vim.Context.t = Vim.key(">");
      let _: Vim.Context.t = Vim.key("g");
      let _: Vim.Context.t = Vim.key("v");
      ();
    });

  let outdentEffect =
    Isolinear.Effect.create(~name="vim.outdent", () => {
      let _: Vim.Context.t = Vim.key("<");
      let _: Vim.Context.t = Vim.key("g");
      let _: Vim.Context.t = Vim.key("v");
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
      let _: Vim.Context.t = Vim.key("<esc>");
      let _: Vim.Context.t = Vim.key("<esc>");
      // Jump to bottom
      let _: Vim.Context.t = Vim.input("g");
      let _: Vim.Context.t = Vim.input("g");
      let _: Vim.Context.t = Vim.input("G");
      let {cursors, topLine: newTopLine, leftColumn: newLeftColumn, _}: Vim.Context.t =
        Vim.input("$");

      // Update the editor, which is the source of truth for cursor position
      let scope = EditorScope.Editor(editorId);
      dispatch(Actions.Editor({scope, msg: CursorsChanged(cursors)}));
      dispatch(Actions.Editor({scope, msg: ScrollToLine(newTopLine - 1)}));
      dispatch(Actions.Editor({scope, msg: ScrollToColumn(newLeftColumn)}));
    });
  };

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | ConfigurationSet(configuration) => (
        state,
        synchronizeViml(configuration),
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

      // Ensure that the editor is focused, as well
      let state'' = state' |> Oni_Model.FocusManager.push(Editor);

      (
        state'',
        openBufferEffect(
          ~onComplete=bufferId => BufferOpened(path, maybeLocation, bufferId),
          path,
        ),
      );
    | BufferOpened(path, maybeLocation, bufferId) =>
      let maybeRenderer =
        switch (Core.BufferPath.parse(path)) {
        | ExtensionDetails => Some(BufferRenderer.ExtensionDetails)
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
        | Image => Some(BufferRenderer.Image)
        | Welcome => Some(BufferRenderer.Welcome)
        | Changelog => Some(BufferRenderer.FullChangelog)
        | FilePath(_) => None
        | DebugInput => Some(BufferRenderer.DebugInput)
        };

      let editor = Feature_Layout.activeEditor(state.layout);
      let editorId = editor |> Editor.getId;

      (
        state,
        Isolinear.Effect.batch([
          maybeRenderer
          |> Option.map(addBufferRendererEffect(bufferId))
          |> Option.value(~default=Isolinear.Effect.none),
          maybeLocation
          |> OptionEx.flatMap(loc =>
               Feature_Editor.Editor.characterToByte(loc, editor)
             )
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
                  if (Editor.getBufferId(editor) == bufferId) {
                    state.buffers
                    |> Feature_Buffers.get(bufferId)
                    |> Option.map(buffer => {
                         let updatedBuffer =
                           buffer
                           |> Oni_Core.Buffer.setFont(state.terminalFont)
                           |> Feature_Editor.EditorBuffer.ofBuffer;
                         Editor.updateBuffer(~buffer=updatedBuffer, editor);
                       })
                    |> Option.value(~default=editor);
                  } else {
                    editor;
                  },
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

    | KeyboardInput({isText, input}) => (state, inputEffect(~isText, input))

    | CopyActiveFilepathToClipboard => (
        state,
        copyActiveFilepathToClipboardEffect,
      )

    | DirectoryChanged(workingDirectory) =>
      let newState = {
        ...state,
        fileExplorer:
          Feature_Explorer.setRoot(
            ~rootPath=workingDirectory,
            state.fileExplorer,
          ),
        workspace: {
          workingDirectory,
          rootName: Filename.basename(workingDirectory),
        },
      };
      (newState, Isolinear.Effect.none);

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

    | TabPage(CloseRelative(delta)) =>
      switch (Feature_Layout.removeLayoutTabRelative(~delta, state.layout)) {
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

    | TabPage(OnlyRelative(count)) => (
        {
          ...state,
          layout:
            state.layout
            |> Feature_Layout.removeOtherLayoutTabsRelative(~count),
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
