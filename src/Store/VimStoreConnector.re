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

type commandLineCompletionMeet = {
  prefix: string,
  position: int,
};

let getCommandLineCompletionsMeet = (str: string, position: int) => {
  let len = String.length(str);

  if (len == 0 || position < len) {
    None;
  } else {
    /* Look backwards for '/' or ' ' */
    let found = ref(false);
    let meet = ref(position);

    while (meet^ > 0 && ! found^) {
      let pos = meet^ - 1;
      let c = str.[pos];
      if (c == ' ') {
        found := true;
      } else {
        decr(meet);
      };
    };

    let pos = meet^;
    Some({prefix: String.sub(str, pos, len - pos), position: pos});
  };
};

let start =
    (
      languageInfo: Ext.LanguageInfo.t,
      getState: unit => State.t,
      getClipboardText,
      setClipboardText,
    ) => {
  let (stream, dispatch) = Isolinear.Stream.create();
  let hasInitialized = ref(false);

  let languageConfigLoader =
    Ext.LanguageConfigurationLoader.create(languageInfo);

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

    let isMultipleLines = s => String.contains(s, '\n');

    let removeTrailingNewLine = s => {
      let len = String.length(s);
      if (len > 0 && s.[len - 1] == '\n') {
        String.sub(s, 0, len - 1);
      } else {
        s;
      };
    };

    let splitNewLines = s =>
      s
      |> removeTrailingNewLine
      |> String.split_on_char('\n')
      |> Array.of_list;

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
      |> Option.map(removeTrailingNewLine)
      |> Option.map(removeWindowsNewLines)
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

  let _: unit => unit =
    Vim.onGoto((_position, _definitionType) => {
      Log.debug("Goto definition requested");
      // Get buffer and cursor position
      let state = getState();
      let maybeBuffer = state |> Selectors.getActiveBuffer;

      let maybeEditor =
        state |> Selectors.getActiveEditorGroup |> Selectors.getActiveEditor;

      let getDefinition = (buffer, editor) => {
        let id = Core.Buffer.getId(buffer);
        let position = Editor.getPrimaryCursor(~buffer, editor);
        Definition.getAt(id, position, state.definition)
        |> Option.map((definitionResult: LanguageFeatures.DefinitionResult.t) => {
             Actions.OpenFileByPath(
               definitionResult.uri |> Core.Uri.toFileSystemPath,
               None,
               Some(definitionResult.location),
             )
           });
      };

      OptionEx.map2(getDefinition, maybeBuffer, maybeEditor)
      |> Option.join
      |> Option.iter(action => dispatch(action));
    });

  let _: unit => unit =
    Vim.Mode.onChanged(newMode => dispatch(Actions.ModeChanged(newMode)));

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
          modified: meta.modified,
          version: meta.version,
        }),
      );
    });

  let _: unit => unit =
    Vim.Buffer.onModifiedChanged((id, modified) => {
      Log.debugf(m => m("Buffer metadata changed: %n | %b", id, modified));
      dispatch(Actions.BufferSetModified(id, modified));
    });

  let _: unit => unit =
    Vim.Buffer.onWrite(id => {dispatch(Actions.BufferSaved(id))});

  let _: unit => unit =
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
            newPosition,
            Location.{line, column},
          ),
        )
      };
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
    Vim.onTerminal(({cmd, curwin, _}) => {
      let splitDirection =
        if (curwin) {Feature_Terminal.Current} else {
          Feature_Terminal.Horizontal
        };

      dispatch(
        Actions.Terminal(Command(NewTerminal({cmd, splitDirection}))),
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
      dispatch(SelectionChanged(vr));
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
        | Vim.Types.TabPage => Actions.OpenFileByPath(buf, None, None)
        };
      dispatch(command);
    });

  let _: unit => unit =
    Vim.Window.onMovement((movementType, _count) => {
      Log.trace("Vim.Window.onMovement");
      let state = getState();

      let move = moveFunc => {
        let maybeEditorGroupId =
          EditorGroups.getActiveEditorGroup(state.editorGroups)
          |> Option.map((group: EditorGroup.t) =>
               moveFunc(group.editorGroupId, state.layout)
             );

        switch (maybeEditorGroupId) {
        | Some(editorGroupId) =>
          dispatch(Actions.EditorGroupSelected(editorGroupId))
        | None => ()
        };
      };

      switch (movementType) {
      | FullLeft
      | OneLeft => move(Feature_Layout.moveLeft)
      | FullRight
      | OneRight => move(Feature_Layout.moveRight)
      | FullDown
      | OneDown => move(Feature_Layout.moveDown)
      | FullUp
      | OneUp => move(Feature_Layout.moveUp)
      | RotateDownwards => dispatch(Actions.Command("view.rotateForward"))
      | RotateUpwards => dispatch(Actions.Command("view.rotateBackward"))
      | TopLeft
      | BottomRight
      | Previous => Log.error("Window movement not implemented")
      };
    });

  let _: unit => unit =
    Vim.Buffer.onEnter(buf => {
      let metadata = Vim.BufferMetadata.ofBuffer(buf);

      if (metadata.id == 1 && ! hasInitialized^) {
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
            modified: metadata.modified,
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
               Actions.BufferUpdate({update: bu, newBuffer, oldBuffer}),
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
        let position = Vim.CommandLine.getPosition();
        let meet = getCommandLineCompletionsMeet(text, position);
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
      Vim.init();

      if (Core.BuildInfo.commitId == Persistence.Global.version()) {
        dispatch(
          Actions.OpenFileByPath(Core.BufferPath.welcome, None, None),
        );
      } else {
        dispatch(
          Actions.OpenFileByPath(Core.BufferPath.updateChangelog, None, None),
        );
      };
      hasInitialized := true;
    });

  let currentBufferId: ref(option(int)) = ref(None);

  let updateActiveEditorCursors = cursors => {
    let () =
      getState()
      |> Selectors.getActiveEditorGroup
      |> Selectors.getActiveEditor
      |> Option.map(Editor.getId)
      |> Option.iter(id => {dispatch(Actions.EditorCursorMove(id, cursors))});
    ();
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

  let inputEffect = key =>
    Isolinear.Effect.create(~name="vim.input", () =>
      if (isVimKey(key)) {
        // Set cursors based on current editor
        let state = getState();
        let editor =
          state |> Selectors.getActiveEditorGroup |> Selectors.getActiveEditor;

        let cursors =
          editor
          |> Option.map(Editor.getVimCursors)
          |> Option.value(~default=[]);

        let primaryCursor =
          switch (cursors) {
          | [hd, ..._] => Some(hd)
          | [] => None
          };

        let () =
          editor
          |> Option.iter(e => {
               let topLine = Editor.getTopVisibleLine(e);
               let leftCol = Editor.getLeftVisibleColumn(e);
               Vim.Window.setTopLeft(topLine, leftCol);
             });

        let syntaxScope =
          state
          |> Selectors.getActiveBuffer
          |> OptionEx.map2(
               (primaryCursor, buffer) => {
                 let bufferId = Core.Buffer.getId(buffer);
                 let {line, column}: Location.t = primaryCursor;

                 Feature_Syntax.getSyntaxScope(
                   ~bufferId,
                   ~line,
                   ~bytePosition=Index.toZeroBased(column),
                   state.syntaxHighlights,
                 );
               },
               primaryCursor,
             )
          |> Option.value(~default=Core.SyntaxScope.none);

        let acpEnabled =
          Core.Configuration.getValue(
            c => c.editorAutoClosingBrackets,
            state.configuration,
          )
          |> (
            fun
            | LanguageDefined => true
            | Never => false
          );

        let autoClosingPairs =
          if (acpEnabled) {
            state
            |> Selectors.getActiveBuffer
            |> OptionEx.flatMap(Core.Buffer.getFileType)
            |> OptionEx.flatMap(
                 Ext.LanguageConfigurationLoader.get_opt(
                   languageConfigLoader,
                 ),
               )
            |> Option.map(
                 Ext.LanguageConfiguration.toVimAutoClosingPairs(syntaxScope),
               );
          } else {
            None;
          };

        let cursors = Vim.input(~autoClosingPairs?, ~cursors, key);
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
        Log.debug("handled key: " ++ key);
      }
    );

  let splitExistingBufferEffect = maybeBuffer =>
    Isolinear.Effect.createWithDispatch(
      ~name="vim.splitExistingBuffer", dispatch => {
      switch (maybeBuffer) {
      | None => Log.warn("Unable to find existing buffer")
      | Some(buffer) =>
        // TODO: Will this be necessary with https://github.com/onivim/oni2/pull/1627?
        let fileType = Core.Buffer.getFileType(buffer);
        let lineEndings = Core.Buffer.getLineEndings(buffer);
        let modified = Core.Buffer.isModified(buffer);
        let filePath = Core.Buffer.getFilePath(buffer);
        let version = Core.Buffer.getVersion(buffer);
        dispatch(
          Actions.BufferEnter({
            id: Oni_Core.Buffer.getId(buffer),
            buffer,
            fileType,
            lineEndings,
            filePath,
            modified,
            version,
          }),
        );
      }
    });

  let openFileByPathEffect = (state, filePath, location) =>
    Isolinear.Effect.create(~name="vim.openFileByPath", () => {
      let buffer = Vim.Buffer.openFile(filePath);
      let metadata = Vim.BufferMetadata.ofBuffer(buffer);
      let lineEndings = Vim.Buffer.getLineEndings(buffer);

      let fileType =
        switch (metadata.filePath) {
        | Some(v) =>
          Some(Ext.LanguageInfo.getLanguageFromFilePath(languageInfo, v))
        | None => None
        };

      let () =
        location
        |> Option.iter((loc: Location.t) => {
             let cursor = (loc :> Vim.Cursor.t);
             let () = updateActiveEditorCursors([cursor]);

             let topLine: int = max(Index.toZeroBased(loc.line) - 10, 0);

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

      let bufferId = Vim.Buffer.getId(buffer);
      let defaultBuffer = Oni_Core.Buffer.ofLines(~id=bufferId, [||]);
      let buffer =
        Selectors.getBufferById(state, bufferId)
        |> Option.value(~default=defaultBuffer);

      // TODO: Will this be necessary with https://github.com/onivim/oni2/pull/1627?
      dispatch(
        Actions.BufferEnter({
          id: metadata.id,
          buffer,
          fileType,
          lineEndings,
          filePath: metadata.filePath,
          modified: metadata.modified,
          version: metadata.version,
        }),
      );

      let maybeRenderer =
        switch (Core.BufferPath.parse(filePath)) {
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

      maybeRenderer
      |> Option.iter(renderer => {
           dispatch(
             Actions.BufferRenderer(RendererAvailable(bufferId, renderer)),
           )
         });
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
      | Some({position, _}) =>
        isCompleting := true;
        let currentPos = ref(Vim.CommandLine.getPosition());
        while (currentPos^ > position) {
          let _ = Vim.input(~cursors=[], "<bs>");
          currentPos := Vim.CommandLine.getPosition();
        };

        let completion = Path.trimTrailingSeparator(completion);
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

        // Set configured line comment
        editorBuffer
        |> OptionEx.flatMap(Core.Buffer.getFileType)
        |> OptionEx.flatMap(
             Ext.LanguageConfigurationLoader.get_opt(languageConfigLoader),
           )
        |> OptionEx.flatMap((config: Ext.LanguageConfiguration.t) =>
             config.lineComment
           )
        |> Option.iter(Vim.Options.setLineComment);

        let synchronizeWindowMetrics = (editor: Editor.t) => {
          let vimWidth = Vim.Window.getWidth();
          let vimHeight = Vim.Window.getHeight();

          let Feature_Editor.EditorLayout.{
                bufferHeightInCharacters: lines,
                bufferWidthInCharacters: columns,
                _,
              } =
            Editor.getLayout(editor);

          if (columns != vimWidth) {
            Vim.Window.setWidth(columns);
          };

          if (lines != vimHeight) {
            Vim.Window.setHeight(lines);
          };
        };

        /* Update the window metrics for the editor */
        /* This synchronizes the window width / height with libvim's model */
        switch (editor) {
        | Some(e) => synchronizeWindowMetrics(e)
        | _ => ()
        };
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
               Vim.command("set paste");
             };

             let latestCursors = ref([]);
             Zed_utf8.iter(
               s => {
                 latestCursors :=
                   Vim.input(~cursors=[], Zed_utf8.singleton(s));
                 ();
               },
               text,
             );

             if (!isCmdLineMode) {
               updateActiveEditorCursors(latestCursors^);
               Vim.command("set nopaste");
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

  let saveEffect =
    Isolinear.Effect.create(~name="vim.save", () => {
      let _ = Vim.input("<esc>");
      let _ = Vim.input("<esc>");
      let _ = Vim.input(":");
      let _ = Vim.input("w");
      let _ = Vim.input("<CR>");
      ();
    });

  let escapeEffect =
    Isolinear.Effect.create(~name="vim.esc", () => {
      let _ = Vim.input("<esc>");
      ();
    });

  let indentEffect =
    Isolinear.Effect.create(~name="vim.indent", () => {
      let _ = Vim.input(">");
      let _ = Vim.input("g");
      let _ = Vim.input("v");
      ();
    });

  let outdentEffect =
    Isolinear.Effect.create(~name="vim.outdent", () => {
      let _ = Vim.input("<");
      let _ = Vim.input("g");
      let _ = Vim.input("v");
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
      let _ = Vim.input("<esc>");
      let _ = Vim.input("<esc>");
      // Jump to bottom
      let _ = Vim.input("g");
      let _ = Vim.input("g");
      let _ = Vim.input("G");
      let cursors = Vim.input("$");
      let newTopLine = Vim.Window.getTopLine();
      let newLeftColumn = Vim.Window.getLeftColumn();

      // Update the editor, which is the source of truth for cursor position
      dispatch(Actions.EditorCursorMove(editorId, cursors));
      dispatch(Actions.EditorScrollToLine(editorId, newTopLine - 1));
      dispatch(Actions.EditorScrollToColumn(editorId, newLeftColumn));
    });
  };

  let addSplit = (direction, state: State.t, editorGroup) => {
    ...state,
    // Fix #686: If we're adding a split, we should turn off Zen mode.
    zenMode: false,
    editorGroups:
      EditorGroups.add(
        ~defaultFont=state.editorFont,
        editorGroup,
        state.editorGroups,
      ),
    layout:
      Feature_Layout.addWindow(
        ~target={
          EditorGroups.getActiveEditorGroup(state.editorGroups)
          |> Option.map((group: EditorGroup.t) => group.editorGroupId);
        },
        ~position=`After,
        direction,
        editorGroup.editorGroupId,
        state.layout,
      ),
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
    | ModeChanged(vimMode) => ({...state, vimMode}, Isolinear.Effect.none)
    | Command("view.splitHorizontal") =>
      let maybeBuffer = Selectors.getActiveBuffer(state);
      let editorGroup = EditorGroup.create();
      let state' = addSplit(`Horizontal, state, editorGroup);
      (state', splitExistingBufferEffect(maybeBuffer));

    | Command("view.splitVertical") =>
      let maybeBuffer = Selectors.getActiveBuffer(state);
      let editorGroup = EditorGroup.create();
      let state' = addSplit(`Vertical, state, editorGroup);
      (state', splitExistingBufferEffect(maybeBuffer));

    | OpenFileByPath(path, maybeDirection, location) =>
      /* If a split was requested, create that first! */
      let state' =
        switch (maybeDirection) {
        | None => state
        | Some(direction) =>
          let editorGroup = EditorGroup.create();
          addSplit(direction, state, editorGroup);
        };
      (state', openFileByPathEffect(state', path, location));
    | BufferEnter(_)
    | EditorFont(Service_Font.FontLoaded(_))
    | EditorGroupSelected(_)
    | EditorSizeChanged(_) => (state, synchronizeEditorEffect(state))
    | BufferSetIndentation(_, indent) => (
        state,
        synchronizeIndentationEffect(indent),
      )
    | ViewSetActiveEditor(_) => (state, synchronizeEditorEffect(state))
    | ViewCloseEditor(_) => (state, synchronizeEditorEffect(state))
    | Command("workbench.action.nextEditor") => (
        state,
        synchronizeEditorEffect(state),
      )
    | Command("workbench.action.previousEditor") => (
        state,
        synchronizeEditorEffect(state),
      )
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

      let maybeEditorId =
        state
        |> Selectors.getActiveEditorGroup
        |> Selectors.getActiveEditor
        |> Option.map((editor: Feature_Editor.Editor.t) => editor.editorId);

      let (state, effect) =
        OptionEx.map3(
          (bufferId, terminalId, editorId) => {
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

            let editorGroups =
              state.editorGroups
              |> EditorGroups.setBufferFont(
                   ~bufferId,
                   ~font=state.terminalFont,
                 );

            (
              {...state, editorGroups, syntaxHighlights},
              setTerminalLinesEffect(~bufferId, ~editorId, lines),
            );
          },
          maybeBufferId,
          maybeTerminalId,
          maybeEditorId,
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
