/*
 * Buffers.re
 *
 * A collection of buffers
 */

open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Model.Buffers"));

type model = IntMap.t(Buffer.t);

let empty = IntMap.empty;

type mapFunction = Buffer.t => Buffer.t;

let map = IntMap.map;
let remove = IntMap.remove;

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
  model |> IntMap.bindings |> List.map(snd) |> List.filter(f);
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

let disableSyntaxHighlighting =
  Option.map(buffer => Buffer.disableSyntaxHighlighting(buffer));

let setModified = modified =>
  Option.map(buffer => Buffer.setModified(modified, buffer));

let setLineEndings = le =>
  Option.map(buffer => Buffer.setLineEndings(le, buffer));

[@deriving show({with_path: false})]
type msg =
  | SyntaxHighlightingDisabled(int)
  | Entered({
      id: int,
      fileType: Oni_Core.Buffer.FileType.t,
      lineEndings: [@opaque] option(Vim.lineEnding),
      filePath: option(string),
      isModified: bool,
      version: int,
      font: Font.t,
      // TODO: This duplication-of-truth is really awkward,
      // but I want to remove it shortly
      buffer: [@opaque] Buffer.t,
    })
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

let update = (msg: msg, model: model) => {
  switch (msg) {
  | SyntaxHighlightingDisabled(id) =>
    IntMap.update(id, disableSyntaxHighlighting, model)

  | Entered({
      id,
      fileType,
      lineEndings,
      isModified,
      filePath,
      version,
      font,
      _,
    }) =>
    let maybeSetLineEndings = (maybeLineEndings, buf) => {
      switch (maybeLineEndings) {
      | Some(le) => Buffer.setLineEndings(le, buf)
      | None => buf
      };
    };

    let updater = (
      fun
      | Some(buffer) =>
        buffer
        |> Buffer.setModified(isModified)
        |> Buffer.setFilePath(filePath)
        |> Buffer.setVersion(version)
        |> Buffer.stampLastUsed
        |> maybeSetLineEndings(lineEndings)
        |> Option.some
      | None =>
        Buffer.ofMetadata(
          ~id,
          ~version,
          ~font,
          ~filePath,
          ~modified=isModified,
        )
        |> Buffer.setFileType(fileType)
        |> Buffer.stampLastUsed
        |> maybeSetLineEndings(lineEndings)
        |> Option.some
    );
    IntMap.update(id, updater, model);

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
    IntMap.update(id, updater, model);
  /* | BufferDelete(bd) => IntMap.remove(bd, state) */
  | ModifiedSet(id, isModified) =>
    IntMap.update(id, setModified(isModified), model)

  | IndentationSet(id, indent) =>
    IntMap.update(id, setIndentation(indent), model)

  | LineEndingsChanged({id, lineEndings}) =>
    IntMap.update(id, setLineEndings(lineEndings), model)

  | Update({update, newBuffer, _}) => IntMap.add(update.id, newBuffer, model)

  | FileTypeChanged({id, fileType}) =>
    IntMap.update(id, Option.map(Buffer.setFileType(fileType)), model)

  | Saved(_) => model
  };
};
