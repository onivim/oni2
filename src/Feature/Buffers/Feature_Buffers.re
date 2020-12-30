/*
 * Buffers.re
 *
 * A collection of buffers
 */

open EditorCoreTypes;
open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Model.Buffers"));

type model = IntMap.t(Buffer.t);

let empty = IntMap.empty;

let map = IntMap.map;

let get = (id, model) => IntMap.find_opt(id, model);

let anyModified = (buffers: model) => {
  IntMap.fold(
    (_key, v, prev) => Buffer.isModified(v) || prev,
    buffers,
    false,
  );
};

let add = (buffer, model) => {
  model |> IntMap.add(Buffer.getId(buffer), buffer);
};

let filter = (f, model) => {
  model |> IntMap.to_seq |> Seq.map(snd) |> Seq.filter(f) |> List.of_seq;
};

let all = model => {
  model |> IntMap.to_seq |> Seq.map(snd) |> List.of_seq;
};

let isModifiedByPath = (buffers: model, filePath: string) => {
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

// TODO: When do we use this?
//let disableSyntaxHighlighting =
//  Option.map(buffer => Buffer.disableSyntaxHighlighting(buffer));

let setModified = modified =>
  Option.map(buffer => Buffer.setModified(modified, buffer));

let setLineEndings = le =>
  Option.map(buffer => Buffer.setLineEndings(le, buffer));

let modified = model => {
  model
  |> IntMap.to_seq
  |> Seq.map(snd)
  |> Seq.filter(Buffer.isModified)
  |> List.of_seq;
};

type outmsg =
  | Nothing
  | BufferUpdated({
      update: Oni_Core.BufferUpdate.t,
      newBuffer: Oni_Core.Buffer.t,
      oldBuffer: Oni_Core.Buffer.t,
      triggerKey: option(string),
    })
  | BufferSaved(Oni_Core.Buffer.t)
  | CreateEditor({
      buffer: Oni_Core.Buffer.t,
      split: [ | `Current | `Horizontal | `Vertical | `NewTab],
      position: option(BytePosition.t),
      grabFocus: bool,
      preview: bool,
    })
  | BufferModifiedSet(int, bool);

[@deriving show]
type command =
  | DetectIndentation;

[@deriving show({with_path: false})]
type msg =
  | Command(command)
  | EditorRequested({
      buffer: [@opaque] Oni_Core.Buffer.t,
      split: [ | `Current | `Horizontal | `Vertical | `NewTab],
      position: option(CharacterPosition.t),
      grabFocus: bool,
      preview: bool,
    })
  | NewBufferAndEditorRequested({
      buffer: [@opaque] Oni_Core.Buffer.t,
      split: [ | `Current | `Horizontal | `Vertical | `NewTab],
      position: option(CharacterPosition.t),
      grabFocus: bool,
      preview: bool,
    })
  //  | SyntaxHighlightingDisabled(int)
  | FileTypeChanged({
      id: int,
      fileType: Oni_Core.Buffer.FileType.t,
    })
  | FilenameChanged({
      id: int,
      newFilePath: option(string),
      newFileType: Oni_Core.Buffer.FileType.t,
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
  | Saved(int)
  | ModifiedSet(int, bool);

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

  let fileNameChanged =
      (~bufferId, ~newFilePath, ~newFileType, ~version, ~isModified) => {
    FilenameChanged({
      id: bufferId,
      newFilePath,
      newFileType,
      version,
      isModified,
    });
  };

  let modified = (~bufferId, ~isModified) => {
    ModifiedSet(bufferId, isModified);
  };

  let updated = (~update, ~newBuffer, ~oldBuffer, ~triggerKey) => {
    Update({update, oldBuffer, newBuffer, triggerKey});
  };
};

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

let update = (~activeBufferId, ~config, msg: msg, model: model) => {
  switch (msg) {
  | EditorRequested({buffer, split, position, grabFocus, preview}) => (
      IntMap.add(Buffer.getId(buffer), buffer, model),
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
      IntMap.add(Buffer.getId(buffer), buffer, model),
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

  | FilenameChanged({id, newFileType, newFilePath, version, isModified}) =>
    let updater = (
      fun
      | Some(buffer) =>
        buffer
        |> Buffer.setModified(isModified)
        |> Buffer.setFilePath(newFilePath)
        |> Buffer.setFileType(newFileType)
        |> Buffer.setVersion(version)
        |> Buffer.stampLastUsed
        |> Option.some
      | None => None
    );
    (IntMap.update(id, updater, model), Nothing);

  | ModifiedSet(id, isModified) => (
      IntMap.update(id, setModified(isModified), model),
      BufferModifiedSet(id, isModified),
    )

  | LineEndingsChanged({id, lineEndings}) => (
      IntMap.update(id, setLineEndings(lineEndings), model),
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
    (
      IntMap.add(update.id, buffer, model),
      BufferUpdated({update, newBuffer: buffer, oldBuffer, triggerKey}),
    );

  | FileTypeChanged({id, fileType}) => (
      IntMap.update(id, Option.map(Buffer.setFileType(fileType)), model),
      Nothing,
    )

  | Saved(bufferId) =>
    let model' =
      IntMap.update(bufferId, Option.map(Buffer.incrementSaveTick), model);
    let eff =
      IntMap.find_opt(bufferId, model)
      |> Option.map(buffer => BufferSaved(buffer))
      |> Option.value(~default=Nothing);
    (model', eff);

  | Command(command) =>
    switch (command) {
    | DetectIndentation =>
      let maybeBuffer = IntMap.find_opt(activeBufferId, model);

      switch (maybeBuffer) {
      // This shouldn't happen...
      | None => (model, Nothing)
      | Some(buffer) =>
        let fileType =
          Buffer.getFileType(buffer) |> Oni_Core.Buffer.FileType.toString;
        let config = config(~fileType);
        let indentation = guessIndentation(~config, buffer);
        let updatedBuffer = Buffer.setIndentation(indentation, buffer);
        (IntMap.add(activeBufferId, updatedBuffer, model), Nothing);
      };
    }
  };
};

module Effects = {
  let openCommon = (~vimBuffer, ~font, ~languageInfo, ~model, f) => {
    let bufferId = Vim.Buffer.getId(vimBuffer);

    switch (IntMap.find_opt(bufferId, model)) {
    // We already have this buffer loaded - so just ask for an editor!
    | Some(buffer) =>
      buffer |> Buffer.stampLastUsed |> f(~alreadyLoaded=true)

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
        |> Buffer.setModified(metadata.modified)
        |> Buffer.stampLastUsed;

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
        ~split=`Current,
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

  let detectIndentation =
    define(
      ~category="Editor",
      ~title="Detect Indentation from Content",
      "editor.action.detectIndentation",
      Command(DetectIndentation),
    );
};

module Contributions = {
  let configuration =
    Configuration.[
      detectIndentation.spec,
      insertSpaces.spec,
      tabSize.spec,
      indentSize.spec,
    ];

  let commands = Commands.[detectIndentation] |> Command.Lookup.fromList;
};
