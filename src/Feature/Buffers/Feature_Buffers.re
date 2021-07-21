/*
 * Buffers.re
 *
 * A collection of buffers
 */

open EditorCoreTypes;
open Oni_Core;
open Utility;

module Log = (val Log.withNamespace("Oni2.Model.Buffers"));

type model = {
  autoSave: AutoSave.model,
  pendingSaveReason: option(SaveReason.t),
  buffers: IntMap.t(Buffer.t),
  originalLines: IntMap.t(array(string)),
  computedDiffs: IntMap.t(DiffMarkers.t),
  checkForLargeFiles: bool,
};

let empty = {
  autoSave: AutoSave.initial,
  pendingSaveReason: None,

  buffers: IntMap.empty,
  originalLines: IntMap.empty,
  computedDiffs: IntMap.empty,

  checkForLargeFiles: true,
};

module Internal = {
  let recomputeDiff = (~bufferId, model) => {
    let computedDiffs' =
      OptionEx.map2(
        (buffer, originalLines) =>
          if (Oni_Core.Buffer.getNumberOfLines(buffer)
              > Constants.diffMarkersMaxLineCount
              || Array.length(originalLines)
              > Constants.diffMarkersMaxLineCount) {
            IntMap.remove(bufferId, model.computedDiffs);
          } else {
            let newMarkers = DiffMarkers.generate(~originalLines, buffer);

            IntMap.add(bufferId, newMarkers, model.computedDiffs);
          },
        IntMap.find_opt(bufferId, model.buffers),
        IntMap.find_opt(bufferId, model.originalLines),
      )
      |> Option.value(~default=model.computedDiffs);

    {...model, computedDiffs: computedDiffs'};
  };
};

let setOriginalLines = (~bufferId, ~originalLines, model) => {
  {
    ...model,
    originalLines: IntMap.add(bufferId, originalLines, model.originalLines),
  }
  |> Internal.recomputeDiff(~bufferId);
};

let getOriginalDiff = (~bufferId, model) => {
  IntMap.find_opt(bufferId, model.computedDiffs);
};

let map = (f, {buffers, _} as model) => {
  ...model,
  buffers: IntMap.map(f, buffers),
};

let get = (id, {buffers, _}) => IntMap.find_opt(id, buffers);

let update = (id, updater, {buffers, _} as model) => {
  {...model, buffers: IntMap.update(id, updater, buffers)};
};

let configurationChanged = (~config, model) => {
  ...model,
  checkForLargeFiles:
    Feature_Configuration.GlobalConfiguration.Editor.largeFileOptimizations.get(
      config,
    ),
  autoSave: AutoSave.configurationChanged(~config, model),
};

let anyModified = ({buffers, _}: model) => {
  IntMap.fold(
    (_key, v, prev) => Buffer.isModified(v) || prev,
    buffers,
    false,
  );
};

let add = (buffer, model) => {
  {
    ...model,
    buffers: model.buffers |> IntMap.add(Buffer.getId(buffer), buffer),
  };
};

let filter = (f, {buffers, _}) => {
  buffers |> IntMap.to_seq |> Seq.map(snd) |> Seq.filter(f) |> List.of_seq;
};

let all = ({buffers, _}) => {
  buffers |> IntMap.to_seq |> Seq.map(snd) |> List.of_seq;
};

let isModifiedByPath = ({buffers, _}: model, filePath: string) => {
  IntMap.exists(
    (_id, v) => {
      let bufferPath = Buffer.getFilePath(v);
      let isModified = Buffer.isModified(v);

      switch (bufferPath) {
      | None => false
      | Some(bp) => String.equal(bp, filePath) && isModified
      };
    },
    buffers,
  );
};

let isLargeFile = (model, buffer) => {
  model.checkForLargeFiles
  && model.buffers
  |> IntMap.find_opt(Oni_Core.Buffer.getId(buffer))
  |> Option.map(buffer =>
       Buffer.getNumberOfLines(buffer) > Constants.largeFileLineCountThreshold
     )
  |> Option.value(~default=false);
};

let setModified = modified =>
  Option.map(buffer => Buffer.setModified(modified, buffer));

let setLineEndings = le =>
  Option.map(buffer => Buffer.setLineEndings(le, buffer));

let modified = model => {
  model.buffers
  |> IntMap.to_seq
  |> Seq.map(snd)
  |> Seq.filter(Buffer.isModified)
  |> List.of_seq;
};

[@deriving show]
type command =
  | ChangeIndentation({mode: IndentationSettings.mode})
  | ChangeFiletype({maybeBufferId: option(int)})
  | ConvertIndentationToTabs
  | ConvertIndentationToSpaces
  | CopyAbsolutePathToClipboard
  | CopyRelativePathToClipboard
  | DetectIndentation
  | SaveWithoutFormatting;

[@deriving show({with_path: false})]
type msg =
  | AutoSave(AutoSave.msg)
  | AutoSaveCompleted
  | Command(command)
  | Noop
  | EditorRequested({
      buffer: [@opaque] Oni_Core.Buffer.t,
      split: SplitDirection.t,
      position: option(CharacterPosition.t),
      grabFocus: bool,
      preview: bool,
    })
  | FocusedBufferChanged({bufferId: int})
  | NewBufferAndEditorRequested({
      buffer: [@opaque] Oni_Core.Buffer.t,
      split: SplitDirection.t,
      position: option(CharacterPosition.t),
      grabFocus: bool,
      preview: bool,
    })
  //  | SyntaxHighlightingDisabled(int)
  | FileTypeChanged({
      id: int,
      fileType: Oni_Core.Buffer.FileType.t,
    })
  | IndentationChanged({
      id: int,
      size: int,
      mode: IndentationSettings.mode,
      didBufferGetModified: bool,
    })
  | IndentationConversionError(string)
  | FilenameChanged({
      id: int,
      newFilePath: option(string),
      version: int,
      isModified: bool,
    })
  | Update({
      update: [@opaque] BufferUpdate.t,
      oldBuffer: [@opaque] Buffer.t,
      newBuffer: [@opaque] Buffer.t,
      triggerKey: option(string),
    })
  | LineEndingsChanged({
      id: int,
      lineEndings: [@opaque] Vim.lineEnding,
    })
  | StatusBarIndentationClicked
  | Saved(int)
  | ModifiedSet(int, bool)
  | LargeFileOptimizationsApplied({
      [@opaque]
      buffer: Buffer.t,
    });

module Msg = {
  let fileTypeChanged = (~bufferId, ~fileType) => {
    FileTypeChanged({id: bufferId, fileType});
  };

  let lineEndingsChanged = (~bufferId, ~lineEndings) => {
    LineEndingsChanged({id: bufferId, lineEndings});
  };

  let saved = (~bufferId) => {
    Saved(bufferId);
  };

  let fileNameChanged = (~bufferId, ~newFilePath, ~version, ~isModified) => {
    FilenameChanged({id: bufferId, newFilePath, version, isModified});
  };

  let modified = (~bufferId, ~isModified) => {
    ModifiedSet(bufferId, isModified);
  };

  let updated = (~update, ~newBuffer, ~oldBuffer, ~triggerKey) => {
    Update({update, oldBuffer, newBuffer, triggerKey});
  };

  let selectFileTypeClicked = (~bufferId: int) =>
    Command(ChangeFiletype({maybeBufferId: Some(bufferId)}));

  let statusBarIndentationClicked = StatusBarIndentationClicked;

  let copyActivePathToClipboard = Command(CopyAbsolutePathToClipboard);
};

let vimSettingChanged = (~activeBufferId, ~name, ~value, model) => {
  let updateTabsOrSpaces = (mode, buffer) => {
    let indentation = Buffer.getIndentation(buffer);
    IndentationChanged({
      id: Oni_Core.Buffer.getId(buffer),
      mode,
      size: indentation.size,
      didBufferGetModified: false,
    });
  };

  let updateShiftWidth = (size, buffer) => {
    let indentation = Buffer.getIndentation(buffer);
    IndentationChanged({
      id: Oni_Core.Buffer.getId(buffer),
      mode: indentation.mode,
      size,
      didBufferGetModified: false,
    });
  };

  let maybeUpdater =
    if (name == "expandtab") {
      Vim.Setting.(
        {
          switch (value) {
          | Int(0) => Some(updateTabsOrSpaces(IndentationSettings.Tabs))
          | Int(1) => Some(updateTabsOrSpaces(IndentationSettings.Spaces))
          | String(_)
          | Int(_) => None
          };
        }
      );
    } else if (name == "shiftwidth") {
      Vim.Setting.(
        {
          switch (value) {
          | Int(size) => Some(updateShiftWidth(size))
          | String(_) => None
          };
        }
      );
    } else {
      None;
    };

  maybeUpdater
  |> OptionEx.flatMap(updater => {
       model.buffers
       |> IntMap.find_opt(activeBufferId)
       |> Option.map(updater)
     })
  |> Option.map(msg =>
       EffectEx.value(
         ~name="Feature_Buffers.Indentation.vimSettingChanged.msg",
         msg,
       )
     )
  |> Option.value(~default=Isolinear.Effect.none);
};

type outmsg =
  | Nothing
  | BufferIndentationChanged({buffer: Oni_Core.Buffer.t})
  | BufferUpdated({
      update: Oni_Core.BufferUpdate.t,
      markerUpdate: Oni_Core.MarkerUpdate.t,
      minimalUpdate: Oni_Core.MinimalUpdate.t,
      newBuffer: Oni_Core.Buffer.t,
      oldBuffer: Oni_Core.Buffer.t,
      triggerKey: option(string),
    })
  | BufferSaved({
      buffer: Oni_Core.Buffer.t,
      reason: SaveReason.t,
    })
  | CreateEditor({
      buffer: Oni_Core.Buffer.t,
      split: SplitDirection.t,
      position: option(BytePosition.t),
      grabFocus: bool,
      preview: bool,
    })
  | BufferModifiedSet(int, bool)
  | SetClipboardText(string)
  | ShowMenu(
      (Exthost.LanguageInfo.t, IconTheme.t) =>
      Feature_Quickmenu.Schema.menu(msg),
    )
  | NotifyInfo(string)
  | NotifyError(string)
  | Effect(Isolinear.Effect.t(msg));

module Configuration = {
  open Config.Schema;

  let detectIndentation =
    setting("editor.detectIndentation", bool, ~default=true);
  let insertSpaces = setting("editor.insertSpaces", bool, ~default=true);
  let indentSize = setting("editor.indentSize", int, ~default=4);
  let tabSize = setting("editor.tabSize", int, ~default=4);
};

let defaultIndentation = (~config) => {
  let insertSpaces = Configuration.insertSpaces.get(config);
  let indentSize = Configuration.indentSize.get(config);
  let tabSize = Configuration.tabSize.get(config);

  IndentationSettings.{
    mode: insertSpaces ? Spaces : Tabs,
    size: indentSize,
    tabSize,
  };
};

let guessIndentation = (~config, buffer) => {
  let defaultTabSize = Configuration.tabSize.get(config);
  let defaultInsertSpaces = Configuration.insertSpaces.get(config);

  let maybeGuess: option(IndentationGuesser.t) =
    IndentationGuesser.guessIndentationArray(
      buffer |> Buffer.getLines,
      defaultTabSize,
      defaultInsertSpaces,
    );

  maybeGuess
  |> Option.map((guess: IndentationGuesser.t) => {
       let size =
         switch (guess.mode) {
         | Tabs => Configuration.tabSize.get(config)
         | Spaces => guess.size
         };

       IndentationSettings.create(~mode=guess.mode, ~size, ~tabSize=size, ())
       |> Inferred.explicit;
     })
  |> Option.value(~default=Inferred.implicit(defaultIndentation(~config)));
};

let update =
    (
      ~activeBufferId,
      ~config,
      ~languageInfo,
      ~workspace,
      msg: msg,
      model: model,
    ) => {
  switch (msg) {
  | AutoSave(msg) =>
    let (autoSave', outmsg) = AutoSave.update(msg, model.autoSave);

    let model' = {...model, autoSave: autoSave'};
    switch (outmsg) {
    | AutoSave.Nothing => (model', Nothing)
    | AutoSave.DoAutoSave => (
        {...model, pendingSaveReason: Some(SaveReason.AutoSave)},
        Effect(Service_Vim.Effects.saveAll(() => AutoSaveCompleted)),
      )
    };

  | AutoSaveCompleted => ({...model, pendingSaveReason: None}, Nothing)

  | EditorRequested({buffer, split, position, grabFocus, preview}) => (
      add(buffer, model),
      CreateEditor({
        buffer,
        split,
        position:
          position
          |> Utility.OptionEx.flatMap(pos =>
               Buffer.characterToBytePosition(pos, buffer)
             ),
        grabFocus,
        preview,
      }),
    )

  | FocusedBufferChanged({bufferId}) =>
    let updater = Option.map(Buffer.stampLastUsed);
    (update(bufferId, updater, model), Nothing);

  | NewBufferAndEditorRequested({
      buffer: originalBuffer,
      split,
      position,
      grabFocus,
      preview,
    }) =>
    let fileType =
      Buffer.getFileType(originalBuffer) |> Oni_Core.Buffer.FileType.toString;
    let config = config(~fileType);
    let buffer =
      if (Configuration.detectIndentation.get(config)
          && !Buffer.isIndentationSet(originalBuffer)) {
        let indentation = guessIndentation(~config, originalBuffer);
        Buffer.setIndentation(indentation, originalBuffer);
      } else {
        let indentation = defaultIndentation(~config);
        Buffer.setIndentation(
          Inferred.implicit(indentation),
          originalBuffer,
        );
      };

    (
      add(buffer, model),
      CreateEditor({
        buffer,
        split,
        position:
          position
          |> Utility.OptionEx.flatMap(pos =>
               Buffer.characterToBytePosition(pos, buffer)
             ),
        grabFocus,
        preview,
      }),
    );

  | FilenameChanged({id, newFilePath, version, isModified}) =>
    let updater = (
      fun
      | Some(buffer) => {
          let buffer' =
            buffer
            |> Buffer.setModified(isModified)
            |> Buffer.setFilePath(newFilePath)
            |> Buffer.setVersion(version);

          let language =
            Exthost.LanguageInfo.getLanguageFromBuffer(languageInfo, buffer');
          buffer'
          |> Buffer.setFileType(Buffer.FileType.inferred(language))
          |> Option.some;
        }
      | None => None
    );
    (update(id, updater, model), Nothing);

  | ModifiedSet(id, isModified) => (
      update(id, setModified(isModified), model),
      BufferModifiedSet(id, isModified),
    )

  | LineEndingsChanged({id, lineEndings}) => (
      update(id, setLineEndings(lineEndings), model),
      Nothing,
    )

  | Update({update, newBuffer, oldBuffer, triggerKey}) =>
    let fileType =
      Buffer.getFileType(newBuffer) |> Oni_Core.Buffer.FileType.toString;
    let config = config(~fileType);
    let buffer =
      if (!Buffer.isIndentationSet(newBuffer)
          && Configuration.detectIndentation.get(config)) {
        let indentation = guessIndentation(~config, newBuffer);
        Buffer.setIndentation(indentation, newBuffer);
      } else {
        newBuffer;
      };
    let minimalUpdate =
      if (update.isFull) {
        MinimalUpdate.fromBuffers(~original=oldBuffer, ~updated=buffer);
      } else {
        MinimalUpdate.fromBufferUpdate(~buffer=oldBuffer, ~update);
      };

    let markerUpdate = MarkerUpdate.create(minimalUpdate);
    (
      add(buffer, model) |> Internal.recomputeDiff(~bufferId=update.id),
      BufferUpdated({
        update,
        newBuffer: buffer,
        oldBuffer,
        triggerKey,
        markerUpdate,
        minimalUpdate,
      }),
    );

  | FileTypeChanged({id, fileType}) => (
      update(id, Option.map(Buffer.setFileType(fileType)), model),
      Nothing,
    )

  | IndentationChanged({id, mode, size, didBufferGetModified}) =>
    let newSettings = IndentationSettings.{mode, size, tabSize: size};
    let model' =
      model
      |> update(
           id,
           Option.map(buffer => {
             let buffer' =
               buffer
               |> Buffer.setIndentation(Inferred.explicit(newSettings));

             if (didBufferGetModified) {
               buffer' |> Buffer.setModified(true);
             } else {
               buffer';
             };
           }),
         );
    let eff =
      IntMap.find_opt(id, model'.buffers)
      |> Option.map(buffer => BufferIndentationChanged({buffer: buffer}))
      |> Option.value(~default=Nothing);

    (model', eff);

  | IndentationConversionError(errorMsg) => (model, NotifyError(errorMsg))

  | Saved(bufferId) =>
    let saveReason =
      model.pendingSaveReason
      |> Option.value(
           ~default=SaveReason.UserInitiated({allowFormatting: true}),
         );
    let model' =
      update(bufferId, Option.map(Buffer.incrementSaveTick), model);
    let eff =
      IntMap.find_opt(bufferId, model.buffers)
      |> Option.map(buffer => BufferSaved({buffer, reason: saveReason}))
      |> Option.value(~default=Nothing);
    (model', eff);

  | StatusBarIndentationClicked =>
    let items = [
      (
        "Indent using spaces...",
        Command(ChangeIndentation({mode: IndentationSettings.Spaces})),
      ),
      (
        "Indent using tabs...",
        Command(ChangeIndentation({mode: IndentationSettings.Tabs})),
      ),
      ("Auto-detect indentation", Command(DetectIndentation)),
      ("Convert indentation to tabs", Command(ConvertIndentationToTabs)),
      ("Convert indentation to spaces", Command(ConvertIndentationToSpaces)),
    ];

    let menuFn =
        (_languageInfo: Exthost.LanguageInfo.t, _iconTheme: IconTheme.t) => {
      Feature_Quickmenu.Schema.menu(
        ~focusFirstItemByDefault=true,
        ~onAccepted=
          (~text as _, ~item) => {
            item |> Option.map(snd) |> Option.value(~default=Noop)
          },
        ~toString=fst,
        items,
      );
    };
    (model, ShowMenu(menuFn));

  | Command(command) =>
    switch (command) {
    | ChangeIndentation({mode}) =>
      let items = List.init(8, idx => idx + 1);
      let menuFn =
          (_languageInfo: Exthost.LanguageInfo.t, _iconTheme: IconTheme.t) => {
        Feature_Quickmenu.Schema.menu(
          ~focusFirstItemByDefault=true,
          ~onAccepted=
            (~text as _, ~item as maybeSize) =>
              maybeSize
              |> Option.map(size => {
                   IndentationChanged({
                     id: activeBufferId,
                     size,
                     mode,
                     didBufferGetModified: false,
                   })
                 })
              |> Option.value(~default=Noop),
          ~toString=string_of_int,
          items,
        );
      };
      (model, ShowMenu(menuFn));

    | ChangeFiletype({maybeBufferId}) =>
      let menuFn =
          (languageInfo: Exthost.LanguageInfo.t, iconTheme: IconTheme.t) => {
        let bufferId = maybeBufferId |> Option.value(~default=activeBufferId);
        let languages =
          languageInfo
          |> Exthost.LanguageInfo.languages
          |> List.map(language =>
               (
                 language,
                 Oni_Core.IconTheme.getIconForLanguage(iconTheme, language),
               )
             );

        let itemToIcon = item => {
          item |> snd |> Option.map(Feature_Quickmenu.Schema.Icon.seti);
        };
        Feature_Quickmenu.Schema.menu(
          ~focusFirstItemByDefault=true,
          ~itemRenderer=
            Feature_Quickmenu.Schema.Renderer.defaultWithIcon(itemToIcon),
          ~onAccepted=
            (~text as _, ~item as maybeLanguage) => {
              maybeLanguage
              |> Option.map(language => {
                   FileTypeChanged({
                     id: bufferId,
                     fileType: Buffer.FileType.explicit(fst(language)),
                   })
                 })
              |> Option.value(~default=Noop)
            },
          ~toString=fst,
          languages,
        );
      };
      (model, ShowMenu(menuFn));

    | ConvertIndentationToSpaces =>
      let maybeBuffer = IntMap.find_opt(activeBufferId, model.buffers);
      switch (maybeBuffer) {
      | None => (model, Nothing)
      | Some(buffer) =>
        let allLines = Buffer.getLines(buffer);
        let indentationSettings = Buffer.getIndentation(buffer);
        let newLines =
          allLines
          |> Array.map(
               IndentationConverter.indentationToSpaces(
                 ~size=indentationSettings.size,
               ),
             );
        (
          model,
          Effect(
            Service_Vim.Effects.setLines(
              ~shouldAdjustCursors=true,
              ~bufferId=activeBufferId,
              ~lines=newLines,
              fun
              | Error(msg) => IndentationConversionError(msg)
              | Ok(_) =>
                IndentationChanged({
                  id: activeBufferId,
                  mode: IndentationSettings.Spaces,
                  size: indentationSettings.size,
                  didBufferGetModified: true,
                }),
            ),
          ),
        );
      };

    | ConvertIndentationToTabs =>
      let maybeBuffer = IntMap.find_opt(activeBufferId, model.buffers);
      switch (maybeBuffer) {
      | None => (model, Nothing)
      | Some(buffer) =>
        let allLines = Buffer.getLines(buffer);
        let indentationSettings = Buffer.getIndentation(buffer);
        let newLines =
          allLines
          |> Array.map(
               IndentationConverter.indentationToTabs(
                 ~size=indentationSettings.size,
               ),
             );
        (
          model,
          Effect(
            Service_Vim.Effects.setLines(
              ~shouldAdjustCursors=true,
              ~bufferId=activeBufferId,
              ~lines=newLines,
              fun
              | Error(msg) => IndentationConversionError(msg)
              | Ok(_) =>
                IndentationChanged({
                  id: activeBufferId,
                  mode: IndentationSettings.Tabs,
                  size: indentationSettings.size,
                  didBufferGetModified: true,
                }),
            ),
          ),
        );
      };

    | CopyAbsolutePathToClipboard =>
      let eff =
        model.buffers
        |> IntMap.find_opt(activeBufferId)
        |> OptionEx.flatMap(Buffer.getFilePath)
        |> Option.map(path => SetClipboardText(path))
        |> Option.value(~default=Nothing);

      (model, eff);

    | CopyRelativePathToClipboard =>
      let maybeBufferPath =
        model.buffers
        |> IntMap.find_opt(activeBufferId)
        |> OptionEx.flatMap(Buffer.getFilePath)
        |> OptionEx.flatMap(Oni_Core.FpExp.absoluteCurrentPlatform);

      let default =
        maybeBufferPath
        |> Option.map(path => SetClipboardText(FpExp.toString(path)))
        |> Option.value(~default=Nothing);

      let maybeWorkspacePath =
        workspace
        |> Feature_Workspace.openedFolder
        |> OptionEx.flatMap(Oni_Core.FpExp.absoluteCurrentPlatform);

      let eff =
        OptionEx.flatMap2(
          (bufferPath, workspacePath) => {
            switch (FpExp.relativize(~source=workspacePath, ~dest=bufferPath)) {
            | Ok(relative) =>
              Some(SetClipboardText(FpExp.relativeToString(relative)))
            | Error(_) => None
            }
          },
          maybeBufferPath,
          maybeWorkspacePath,
        )
        |> Option.value(~default);
      (model, eff);

    | DetectIndentation =>
      let maybeBuffer = IntMap.find_opt(activeBufferId, model.buffers);

      switch (maybeBuffer) {
      // This shouldn't happen...
      | None => (model, Nothing)
      | Some(buffer) =>
        let fileType =
          Buffer.getFileType(buffer) |> Oni_Core.Buffer.FileType.toString;
        let config = config(~fileType);
        let indentation = guessIndentation(~config, buffer);
        let updatedBuffer = Buffer.setIndentation(indentation, buffer);
        (add(updatedBuffer, model), Nothing);
      };

    | SaveWithoutFormatting => (
        {
          ...model,
          pendingSaveReason:
            Some(SaveReason.UserInitiated({allowFormatting: false})),
        },
        Effect(
          Service_Vim.Effects.save(~bufferId=activeBufferId, () =>
            Saved(activeBufferId)
          ),
        ),
      )
    }

  | LargeFileOptimizationsApplied({buffer}) =>
    let maybeFilename = Oni_Core.Buffer.getShortFriendlyName(buffer);
    let outmsg =
      maybeFilename
      |> Option.map(fileName => {
           let msg =
             Printf.sprintf(
               "Syntax highlighting and other features have been turned off for the large file '%s'. These optimizations can be disabled by setting 'editor.largeFileOptimizations' to false.",
               fileName,
             );
           NotifyInfo(msg);
         })
      |> Option.value(~default=Nothing);
    (model, outmsg);

  | Noop => (model, Nothing)
  };
};

module Effects = {
  let openCommon = (~vimBuffer, ~font, ~languageInfo, ~model, f) => {
    let bufferId = Vim.Buffer.getId(vimBuffer);

    switch (IntMap.find_opt(bufferId, model.buffers)) {
    // We already have this buffer loaded - so just ask for an editor!
    | Some(buffer) => buffer |> f(~alreadyLoaded=true)

    | None =>
      // No buffer yet, so we need to create one _and_ ask for an editor.
      let metadata = Vim.BufferMetadata.ofBuffer(vimBuffer);
      let maybeLineEndings: option(Vim.lineEnding) =
        Vim.Buffer.getLineEndings(vimBuffer);

      let lines = Vim.Buffer.getLines(vimBuffer);
      let buffer =
        Oni_Core.Buffer.ofLines(~id=metadata.id, ~font, lines)
        |> Buffer.setVersion(metadata.version)
        |> Buffer.setFilePath(metadata.filePath)
        |> Buffer.setModified(metadata.modified);

      let fileType =
        Exthost.LanguageInfo.getLanguageFromBuffer(languageInfo, buffer);

      let buffer =
        maybeLineEndings
        |> Option.map(le => Buffer.setLineEndings(le, buffer))
        |> Option.value(~default=buffer)
        |> Buffer.setFileType(Buffer.FileType.inferred(fileType));

      f(~alreadyLoaded=false, buffer);
    };
  };

  let loadFile = (~font, ~languageInfo, ~filePath, ~toMsg, model) => {
    Isolinear.Effect.createWithDispatch(
      ~name="Feature_Buffers.loadFile", dispatch => {
      let handler = (~alreadyLoaded as _, buffer) => {
        let lines = Oni_Core.Buffer.getLines(buffer);
        dispatch(toMsg(lines));
      };

      let newBuffer = Vim.Buffer.openFile(filePath);

      openCommon(~vimBuffer=newBuffer, ~languageInfo, ~font, ~model, handler);
    });
  };

  let openFileInEditor =
      (
        ~font: Service_Font.font,
        ~languageInfo: Exthost.LanguageInfo.t,
        ~split=SplitDirection.Current,
        ~position=None,
        ~grabFocus=true,
        ~filePath,
        ~preview=false,
        model,
      ) => {
    Isolinear.Effect.createWithDispatch(
      ~name="Feature_Buffers.openFileInEditor", dispatch => {
      let newBuffer = Vim.Buffer.openFile(filePath);

      let handler = (~alreadyLoaded, buffer) =>
        if (alreadyLoaded) {
          dispatch(
            EditorRequested({buffer, split, position, grabFocus, preview}),
          );
        } else {
          dispatch(
            NewBufferAndEditorRequested({
              buffer,
              split,
              position,
              grabFocus,
              preview,
            }),
          );
        };

      openCommon(~vimBuffer=newBuffer, ~languageInfo, ~font, ~model, handler);
    });
  };

  let openNewBuffer = (~font, ~languageInfo, ~split, model) => {
    Isolinear.Effect.createWithDispatch(
      ~name="Feature_Buffers.openNewBuffer", dispatch => {
      let buffer = Vim.Buffer.make();

      let handler = (~alreadyLoaded as _, buffer) =>
        dispatch(
          NewBufferAndEditorRequested({
            buffer,
            split,
            position: None,
            grabFocus: true,
            preview: false,
          }),
        );

      openCommon(~vimBuffer=buffer, ~languageInfo, ~font, ~model, handler);
    });
  };

  let openBufferInEditor = (~font, ~languageInfo, ~split, ~bufferId, model) => {
    Isolinear.Effect.createWithDispatch(
      ~name="Feature_Buffers.openBufferInEditor", dispatch => {
      switch (Vim.Buffer.getById(bufferId)) {
      | None => Log.warnf(m => m("Unable to open buffer: %d", bufferId))
      | Some(vimBuffer) =>
        let handler = (~alreadyLoaded, buffer) => {
          let position = None;
          let grabFocus = true;

          if (alreadyLoaded) {
            dispatch(
              EditorRequested({
                buffer,
                split,
                position,
                grabFocus,
                preview: false,
              }),
            );
          } else {
            dispatch(
              NewBufferAndEditorRequested({
                buffer,
                split,
                position,
                grabFocus,
                preview: false,
              }),
            );
          };
        };

        openCommon(~vimBuffer, ~languageInfo, ~font, ~model, handler);
      }
    });
  };
};

module Commands = {
  open Feature_Commands.Schema;

  let indentUsingTabs =
    define(
      ~title="Indent using tabs",
      "editor.action.indentUsingTabs",
      Command(ChangeIndentation({mode: IndentationSettings.Tabs})),
    );

  let indentUsingSpaces =
    define(
      ~title="Indent using spaces",
      "editor.action.indentUsingSpaces",
      Command(ChangeIndentation({mode: IndentationSettings.Spaces})),
    );

  let detectIndentation =
    define(
      ~category="Editor",
      ~title="Detect Indentation from Content",
      "editor.action.detectIndentation",
      Command(DetectIndentation),
    );

  let convertIndentationToSpaces =
    define(
      ~title="Convert indentation to spaces",
      "editor.action.indentationToSpaces",
      Command(ConvertIndentationToSpaces),
    );

  let convertIndentationToTabs =
    define(
      ~title="Convert indentation to tabs",
      "editor.action.indentationToTabs",
      Command(ConvertIndentationToTabs),
    );

  let changeFiletype =
    define(
      ~category="Editor",
      ~title="Select file type",
      "workbench.action.editor.changeLanguageMode",
      Command(ChangeFiletype({maybeBufferId: None})),
    );

  let copyAbsolutePath =
    define(
      ~category="File",
      ~title="Copy Path of Active File",
      "copyFilePath",
      Command(CopyAbsolutePathToClipboard),
    );
  let copyRelativePath =
    define(
      ~category="File",
      ~title="Copy Relative Path of Active File",
      "copyRelativeFilePath",
      Command(CopyRelativePathToClipboard),
    );

  let saveWithoutFormatting =
    define(
      ~category="File",
      ~title="Save without formatting",
      "workbench.action.files.saveWithoutFormatting",
      Command(SaveWithoutFormatting),
    );
};

let sub = (~isWindowFocused, ~maybeFocusedBuffer, model) => {
  let buffers = model.buffers |> IntMap.bindings |> List.map(snd);
  let largeFileSub =
    if (!model.checkForLargeFiles) {
      Isolinear.Sub.none;
    } else {
      buffers
      |> List.filter((buffer: Buffer.t) => isLargeFile(model, buffer))
      |> List.map(buffer =>
           SubEx.value(
             ~uniqueId=
               "Feature_Buffers.largeFile:"
               ++ string_of_int(Buffer.getId(buffer)),
             LargeFileOptimizationsApplied({buffer: buffer}),
           )
         )
      |> Isolinear.Sub.batch;
    };

  let focusedBufferSub =
    switch (maybeFocusedBuffer) {
    | None => Isolinear.Sub.none
    | Some(bufferId) =>
      let uniqueId =
        "Feature_Buffers.Sub.focusedBuffer:" ++ string_of_int(bufferId);
      SubEx.value(~uniqueId, FocusedBufferChanged({bufferId: bufferId}));
    };

  let autoSaveSub =
    AutoSave.sub(
      ~isWindowFocused,
      ~maybeFocusedBuffer,
      ~buffers,
      model.autoSave,
    )
    |> Isolinear.Sub.map(msg => AutoSave(msg));
  [largeFileSub, autoSaveSub, focusedBufferSub] |> Isolinear.Sub.batch;
};

module Contributions = {
  let configuration =
    Configuration.[
      detectIndentation.spec,
      insertSpaces.spec,
      tabSize.spec,
      indentSize.spec,
    ]
    @ AutoSave.Contributions.configuration;

  let commands =
    Commands.[
      changeFiletype,
      convertIndentationToSpaces,
      convertIndentationToTabs,
      copyAbsolutePath,
      copyRelativePath,
      detectIndentation,
      indentUsingSpaces,
      indentUsingTabs,
      saveWithoutFormatting,
    ]
    |> Command.Lookup.fromList;

  let keybindings =
    Feature_Input.Schema.[
      bind(
        ~key="<D-N>",
        ~command=":enew",
        ~condition="isMac" |> WhenExpr.parse,
      ),
      bind(
        ~key="<D-S>",
        ~command=":w!",
        ~condition="isMac" |> WhenExpr.parse,
      ),
      bind(
        ~key="<D-S-S>",
        ~command=":wa!",
        ~condition="isMac" |> WhenExpr.parse,
      ),
    ];
};
