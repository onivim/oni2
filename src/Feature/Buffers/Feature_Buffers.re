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

let setIndentation = indent =>
  Option.map(buffer => Buffer.setIndentation(indent, buffer));

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
    })
  | CreateEditor({
      buffer: Oni_Core.Buffer.t,
      split: [ | `Current | `Horizontal | `Vertical | `NewTab],
      position: option(BytePosition.t),
      grabFocus: bool,
    });

[@deriving show({with_path: false})]
type msg =
  | EditorRequested({
      buffer: [@opaque] Oni_Core.Buffer.t,
      split: [ | `Current | `Horizontal | `Vertical | `NewTab],
      position: option(CharacterPosition.t),
      grabFocus: bool,
    })
  | NewBufferAndEditorRequested({
      buffer: [@opaque] Oni_Core.Buffer.t,
      split: [ | `Current | `Horizontal | `Vertical | `NewTab],
      position: option(CharacterPosition.t),
      grabFocus: bool,
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
  | IndentationSet(int, [@opaque] IndentationSettings.t)
  | ModifiedSet(int, bool);

module Msg = {
  let fileTypeChanged = (~bufferId, ~fileType) => {
    FileTypeChanged({id: bufferId, fileType});
  };

  let lineEndingsChanged = (~bufferId, ~lineEndings) => {
    LineEndingsChanged({id: bufferId, lineEndings});
  };

  let indentationSet = (~bufferId, ~indentation) => {
    IndentationSet(bufferId, indentation);
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

let update = (msg: msg, model: model) => {
  switch (msg) {
  | EditorRequested({buffer, split, position, grabFocus}) => (
      model,
      CreateEditor({
        buffer,
        split,
        position:
          position
          |> Utility.OptionEx.flatMap(pos =>
               Buffer.characterToBytePosition(pos, buffer)
             ),
        grabFocus,
      }),
    )

  | NewBufferAndEditorRequested({buffer, split, position, grabFocus}) => (
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
      }),
    )

  //  | Entered({
  //      id,
  //      fileType,
  //      lineEndings,
  //      isModified,
  //      filePath,
  //      version,
  //      font,
  //      _,
  //    }) =>
  //    let maybeSetLineEndings = (maybeLineEndings, buf) => {
  //      switch (maybeLineEndings) {
  //      | Some(le) => Buffer.setLineEndings(le, buf)
  //      | None => buf
  //      };
  //    };
  //
  //    let updater = (
  //      fun
  //      | Some(buffer) =>
  //        buffer
  //        |> Buffer.setModified(isModified)
  //        |> Buffer.setFilePath(filePath)
  //        |> Buffer.setVersion(version)
  //        |> Buffer.stampLastUsed
  //        |> maybeSetLineEndings(lineEndings)
  //        |> Option.some
  //      | None =>
  //        Buffer.ofMetadata(
  //          ~id,
  //          ~version,
  //          ~font,
  //          ~filePath,
  //          ~modified=isModified,
  //        )
  //        |> Buffer.setFileType(fileType)
  //        |> Buffer.stampLastUsed
  //        |> maybeSetLineEndings(lineEndings)
  //        |> Option.some
  //    );
  //    (IntMap.update(id, updater, model), Nothing);

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
  /* | BufferDelete(bd) => IntMap.remove(bd, state) */
  | ModifiedSet(id, isModified) => (
      IntMap.update(id, setModified(isModified), model),
      Nothing,
    )

  | IndentationSet(id, indent) => (
      IntMap.update(id, setIndentation(indent), model),
      Nothing,
    )

  | LineEndingsChanged({id, lineEndings}) => (
      IntMap.update(id, setLineEndings(lineEndings), model),
      Nothing,
    )

  | Update({update, newBuffer, _}) => (
      IntMap.add(update.id, newBuffer, model),
      BufferUpdated({update, newBuffer}),
    )

  | FileTypeChanged({id, fileType}) => (
      IntMap.update(id, Option.map(Buffer.setFileType(fileType)), model),
      Nothing,
    )

  | Saved(_) => (model, Nothing)
  };
};

module Effects = {
  let openInEditor =
      (
        ~font: Service_Font.font,
        ~languageInfo: Exthost.LanguageInfo.t,
        ~split=`Current,
        ~position=None,
        ~grabFocus=true,
        ~filePath,
        model,
      ) => {
    //let currentBuffer = Vim.Buffer.getCurrent();
    Isolinear.Effect.createWithDispatch(
      ~name="Feature_Buffers.openInEditor", dispatch => {
      let newBuffer = Vim.Buffer.openFile(filePath);

      let bufferId = Vim.Buffer.getId(newBuffer);

      switch (IntMap.find_opt(bufferId, model)) {
      // We already have this buffer loaded - so just ask for an editor!
      | Some(buffer) =>
        dispatch(EditorRequested({buffer, split, position, grabFocus}))

      | None =>
        // No buffer yet, so we need to create one _and_ ask for an editor.
        let metadata = Vim.BufferMetadata.ofBuffer(newBuffer);
        let maybeLineEndings: option(Vim.lineEnding) =
          Vim.Buffer.getLineEndings(newBuffer);

        let lines = Vim.Buffer.getLines(newBuffer);
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

        dispatch(
          NewBufferAndEditorRequested({buffer, split, position, grabFocus}),
        );
      };
    });
  };
};
