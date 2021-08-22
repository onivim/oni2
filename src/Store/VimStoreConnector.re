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
module Editor = Feature_Editor.Editor;

module Log = (val Core.Log.withNamespace("Oni2.Store.Vim"));

let vimFileWatcherKey =
  Service_FileWatcher.Key.create(
    ~friendlyName="VimStoreConnector.FileWatcher",
  );

let start =
    (
      ~showUpdateChangelog: bool,
      getState: unit => State.t,
      getClipboardText,
      setClipboardText,
    ) => {
  let (stream, dispatch) = Isolinear.Stream.create();
  let libvimHasInitialized = ref(false);
  let currentTriggerKey = ref(None);

  Vim.Clipboard.setProvider(reg => {
    let state = getState();

    let yankConfig = Feature_Vim.useSystemClipboard(state.vim);

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
      Actions.OpenFileByPath(
        "oni://Version",
        Core.SplitDirection.Current,
        None,
      )
      |> dispatch
    });

  let _: unit => unit =
    Vim.Buffer.onLineEndingsChanged((id, lineEndings) => {
      Actions.Buffers(
        Feature_Buffers.Msg.lineEndingsChanged(~bufferId=id, ~lineEndings),
      )
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
            Feature_Buffers.Msg.fileTypeChanged(
              ~bufferId=id,
              ~fileType=Oni_Core.Buffer.FileType.explicit(fileType),
            ),
          ),
        );
      };
    });

  let handleSplit = (split: Vim.Split.t) => {
    let currentBufferId = Vim.Buffer.getId(Vim.Buffer.getCurrent());

    let actionForFilePath = (filePath, direction) => {
      switch (filePath) {
      | Some(fp) => Actions.OpenFileByPath(fp, direction, None)
      // No file path specified, so let's use the current buffer
      | None => Actions.OpenBufferById({bufferId: currentBufferId, direction})
      };
    };
    Vim.Split.(
      switch (split) {
      | NewHorizontal =>
        Actions.NewBuffer({direction: Core.SplitDirection.Horizontal})
      | NewVertical =>
        Actions.NewBuffer({
          direction: Core.SplitDirection.Vertical({shouldReuse: false}),
        })
      | NewTabPage =>
        Actions.NewBuffer({direction: Core.SplitDirection.NewTab})
      | Vertical({filePath}) =>
        actionForFilePath(
          filePath,
          Core.SplitDirection.Vertical({shouldReuse: false}),
        )
      | Horizontal({filePath}) =>
        actionForFilePath(filePath, Core.SplitDirection.Horizontal)
      | TabPage({filePath}) =>
        actionForFilePath(filePath, Core.SplitDirection.NewTab)
      }
    );
  };

  let handleGoto = gotoType => {
    switch (gotoType) {
    | Vim.Goto.Hover =>
      dispatch(
        Actions.LanguageSupport(Feature_LanguageSupport.Msg.Hover.show),
      )

    | Vim.Goto.Outline =>
      dispatch(Actions.SideBar(Feature_SideBar.(Command(GotoOutline))))

    | Vim.Goto.Messages =>
      dispatch(
        Actions.Pane(
          Feature_Pane.Msg.toggle(~paneId="workbench.panel.notifications"),
        ),
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
               Core.SplitDirection.Current,
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
      // This is handled by the returned `effects` list -
      // ideally, all the commands here could be factored to be handled in the same way
      | Scroll(_) => ()
      | SearchStringChanged(_) => ()
      | SearchClearHighlights => ()

      | Output({cmd, output, isSilent}) => {
          dispatch(
            Actions.Vim(Feature_Vim.Msg.output(~cmd, ~output, ~isSilent)),
          );
        }

      | Clear({target, count}) =>
        Vim.Clear.(
          {
            switch (target) {
            | Messages =>
              dispatch(
                Actions.Notification(Feature_Notification.Msg.clear(count)),
              )
            };
          }
        )
      | Map(mapping) =>
        dispatch(Actions.Input(Feature_Input.Msg.vimMap(mapping)))
      | Unmap({mode, keys}) =>
        dispatch(Actions.Input(Feature_Input.Msg.vimUnmap(mode, keys)))

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
      | SettingChanged(setting) => {
          dispatch(Actions.Vim(Feature_Vim.Msg.settingChanged(~setting)));
        }

      | ColorSchemeChanged(maybeColorScheme) =>
        switch (maybeColorScheme) {
        | None => dispatch(Actions.Theme(Feature_Theme.Msg.openThemePicker))
        | Some(colorScheme) =>
          dispatch(
            Actions.Theme(
              Feature_Theme.Msg.vimColorSchemeSelected(~themeId=colorScheme),
            ),
          )
        }

      | MacroRecordingStarted(_) => ()

      | MacroRecordingStopped(_) => ()

      | WindowSplit(split) => handleSplit(split) |> dispatch,
    );

  let _: unit => unit =
    Vim.onDirectoryChanged(newDir => {
      dispatch(
        Actions.Workspace(
          Feature_Workspace.Msg.workingDirectoryChanged(newDir),
        ),
      )
    });

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

      let yankConfig = Feature_Vim.useSystemClipboard(state.vim);
      let allYanks = yankConfig.yank;
      let allDeletes = yankConfig.delete;
      let isClipboardRegister = register == '*' || register == '+';
      let shouldPropagateToClipboard =
        isClipboardRegister
        || operator == Vim.Yank.Yank
        && allYanks
        || (operator == Vim.Yank.Delete || operator == Vim.Yank.Change)
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
      dispatch(
        Actions.Buffers(
          Feature_Buffers.Msg.fileNameChanged(
            ~bufferId=meta.id,
            ~newFilePath=meta.filePath,
            ~isModified=meta.modified,
            ~version=meta.version,
          ),
        ),
      );
    });

  let _: unit => unit =
    Vim.Buffer.onModifiedChanged((id, isModified) => {
      Log.debugf(m => m("Buffer metadata changed: %n | %b", id, isModified));
      dispatch(
        Actions.Buffers(
          Feature_Buffers.Msg.modified(~bufferId=id, ~isModified),
        ),
      );
    });

  let _: unit => unit =
    Vim.Buffer.onWrite(id => {
      dispatch(Actions.Buffers(Feature_Buffers.Msg.saved(~bufferId=id)))
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
        if (curwin) {Oni_Core.SplitDirection.Current} else {
          Oni_Core.SplitDirection.Horizontal
        };

      dispatch(
        Actions.Terminal(
          Feature_Terminal.Msg.terminalCreatedFromVim(
            ~cmd,
            ~splitDirection,
            ~closeOnExit=closeOnFinish,
          ),
        ),
      );
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
          ~shouldAdjustCursorPosition=update.shouldAdjustCursorPosition,
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
        let languageInfo =
          getState().languageSupport |> Feature_LanguageSupport.languageInfo;
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
                 let language =
                   Exthost.LanguageInfo.getLanguageFromBuffer(
                     languageInfo,
                     newBuffer,
                   );
                 let fileType = Oni_Core.Buffer.FileType.inferred(language);

                 newBuffer |> Core.Buffer.setFileType(fileType);
               } else {
                 newBuffer;
               };

             dispatch(
               Actions.Buffers(
                 Feature_Buffers.Msg.updated(
                   ~update=bu,
                   ~newBuffer,
                   ~oldBuffer,
                   ~triggerKey=currentTriggerKey^,
                 ),
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

  let checkCommandLineCompletions = (~text: string, ~position: int) => {
    Log.debug("checkCommandLineCompletions");

    if (position == String.length(text) && !StringEx.isEmpty(text)) {
      let context = Oni_Model.VimContext.current(getState());
      let completions = Vim.CommandLine.getCompletions(~context, ());

      let completions =
        if (StringEx.startsWith(~prefix="set no", text)) {
          completions |> Array.map(name => "no" ++ name);
        } else {
          completions;
        };

      Log.debugf(m => m("  got %n completions.", Array.length(completions)));

      let items =
        Array.map(
          name => {
            Actions.{
              name,
              category: None,
              icon: None,
              command: _ => Noop,
              highlight: [],
            }
          },
          completions,
        );

      dispatch(Actions.QuickmenuUpdateFilterProgress(items, Complete));
    };
  };

  let _: unit => unit =
    Vim.CommandLine.onUpdate(({text, position: cursorPosition, cmdType}) => {
      dispatch(Actions.QuickmenuCommandlineUpdated(text, cursorPosition));

      let cmdlineType = cmdType;
      switch (cmdlineType) {
      | Ex =>
        let meet = Feature_Vim.CommandLine.getCompletionMeet(text);
        lastCompletionMeet := meet;

        isCompleting^
          ? () : checkCommandLineCompletions(~position=cursorPosition, ~text);

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
          Actions.OpenFileByPath(
            Core.BufferPath.updateChangelog,
            Core.SplitDirection.Current,
            None,
          ),
        );
      };
      libvimHasInitialized := true;
    });

  let updateActiveEditorMode = (~allowAnimation=true, subMode, mode, effects) => {
    dispatch(
      // TODO
      Actions.Vim(
        Feature_Vim.Msg.modeChanged(
          ~allowAnimation,
          ~subMode,
          ~mode,
          ~effects,
        ),
      ),
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

  let commandEffect = (~allowAnimation, cmd) => {
    Isolinear.Effect.create(~name="vim.command", () => {
      let state = getState();
      let prevContext = Oni_Model.VimContext.current(state);
      let (newContext, effects) = Vim.command(~context=prevContext, cmd);

      if (newContext.bufferId != prevContext.bufferId) {
        dispatch(
          Actions.OpenBufferById({
            bufferId: newContext.bufferId,
            direction: Core.SplitDirection.Current,
          }),
        );
      } else {
        updateActiveEditorMode(
          ~allowAnimation,
          newContext.subMode,
          newContext.mode,
          effects,
        );
      };
    });
  };

  let inputEffect = (~isText, key) =>
    Isolinear.Effect.createWithDispatch(~name="vim.input", dispatch =>
      if (isVimKey(key)) {
        // Set cursors based on current editor
        let state = getState();
        // let editorId =
        //   Feature_Layout.activeEditor(state.layout) |> Editor.getId;

        let context = Oni_Model.VimContext.current(state);
        let previousBufferId = context.bufferId;

        currentTriggerKey := Some(key);
        let ({mode, bufferId, subMode, _}: Vim.Context.t, effects) =
          isText ? Vim.input(~context, key) : Vim.key(~context, key);
        currentTriggerKey := None;

        // If we switched buffer, open it in current editor
        if (previousBufferId != bufferId) {
          dispatch(
            Actions.OpenBufferById({
              bufferId,
              direction: Core.SplitDirection.Current,
            }),
          );
        };

        updateActiveEditorMode(subMode, mode, effects);
        Log.debug("handled key: " ++ key);
      }
    );

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
          let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.key("<bs>");
          currentPos := Vim.CommandLine.getPosition();
        };

        let completion =
          completion |> Path.trimTrailingSeparator |> StringEx.escapeSpaces;
        let (latestContext: Vim.Context.t, effects) =
          Core.VimEx.inputString(completion);
        updateActiveEditorMode(
          latestContext.subMode,
          latestContext.mode,
          effects,
        );
        isCompleting := false;
      }
    );

  let prevViml = ref([]);
  let synchronizeViml = lines =>
    Isolinear.Effect.create(~name="vim.synchronizeViml", () =>
      if (prevViml^ != lines) {
        let linesArray = Array.of_list(lines);
        Log.info("Running VimL from config...");
        Vim.commands(linesArray) |> ignore;
        Log.info("VimL command completed.");
        prevViml := lines;
      }
    );

  let undoEffect =
    Isolinear.Effect.create(~name="vim.undo", () => {
      let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.key("<esc>");
      let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.key("<esc>");
      let ({subMode, mode, _}: Vim.Context.t, effects) = Vim.key("u");
      updateActiveEditorMode(subMode, mode, effects);
      ();
    });

  let redoEffect =
    Isolinear.Effect.create(~name="vim.redo", () => {
      let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.key("<esc>");
      let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.key("<esc>");
      let ({subMode, mode, _}: Vim.Context.t, effects) = Vim.key("<c-r>");
      updateActiveEditorMode(subMode, mode, effects);
      ();
    });

  let saveEffect =
    Isolinear.Effect.create(~name="vim.save", () => {
      let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.key("<esc>");
      let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.key("<esc>");
      let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.key(":");
      let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.key("w");
      let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.key("<CR>");
      ();
    });

  let escapeEffect =
    Isolinear.Effect.create(~name="vim.esc", () => {
      let ({subMode, mode, _}: Vim.Context.t, effects) = Vim.key("<esc>");
      updateActiveEditorMode(subMode, mode, effects);
      ();
    });

  let indentEffect =
    Isolinear.Effect.create(~name="vim.indent", () => {
      let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.key(">");
      let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.key("g");
      let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.key("v");
      ();
    });

  let outdentEffect =
    Isolinear.Effect.create(~name="vim.outdent", () => {
      let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.key("<");
      let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.key("g");
      let _: (Vim.Context.t, list(Vim.Effect.t)) = Vim.key("v");
      ();
    });

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | SynchronizeExperimentalViml(lines) => (state, synchronizeViml(lines))

    | CommandInvoked({command, _}) =>
      switch (command) {
      | "undo" => (state, undoEffect)
      | "redo" => (state, redoEffect)
      | "workbench.action.files.save" => (state, saveEffect)
      | "indent" => (state, indentEffect)
      | "outdent" => (state, outdentEffect)
      | "editor.action.indentLines" => (state, indentEffect)
      | "editor.action.outdentLines" => (state, outdentEffect)
      | "vim.esc" => (state, escapeEffect)
      | "vim.tutor" => (state, openTutorEffect)
      | _ => (state, Isolinear.Effect.none)
      }
    | VimExecuteCommand({allowAnimation, command}) => (
        state,
        commandEffect(~allowAnimation, command),
      )

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
    | KeyboardInput({isText, input}) => (state, inputEffect(~isText, input))

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
            Core.Buffer.getFilePath(buffer)
            == Some(Core.FpExp.toString(event.watchedPath))
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
  |> Feature_Buffers.all
  |> List.filter_map(buffer =>
       buffer
       |> Core.Buffer.getFilePath
       |> OptionEx.flatMap(Core.FpExp.absoluteCurrentPlatform)
       |> Option.map(path =>
            Service_FileWatcher.watch(
              ~watchChanges=true, ~key=vimFileWatcherKey, ~path, ~onEvent=event =>
              Actions.FileChanged(event)
            )
          )
     );
};
