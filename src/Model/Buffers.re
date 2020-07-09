/*
 * Buffers.re
 *
 * A collection of buffers
 */

open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Model.Buffers"));

type t = IntMap.t(Buffer.t);

let empty = IntMap.empty;

type mapFunction = Buffer.t => Buffer.t;

let map = IntMap.map;
let update = IntMap.update;
let remove = IntMap.remove;

let getBuffer = (id, map) => IntMap.find_opt(id, map);

let anyModified = (buffers: t) => {
  IntMap.fold(
    (_key, v, prev) => Buffer.isModified(v) || prev,
    buffers,
    false,
  );
};

let isModifiedByPath = (buffers: t, filePath: string) => {
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

let reduce = (state: t, action: Actions.t) => {
  switch (action) {
  | BufferDisableSyntaxHighlighting(id) =>
    IntMap.update(id, disableSyntaxHighlighting, state)

  | BufferEnter({id, fileType, lineEndings, isModified, filePath, version, _}) =>
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
        Buffer.ofMetadata(~id, ~version, ~filePath, ~modified=isModified)
        |> Buffer.setFileType(fileType)
        |> Buffer.stampLastUsed
        |> maybeSetLineEndings(lineEndings)
        |> Option.some
    );
    IntMap.update(id, updater, state);

  | BufferFilenameChanged({id, newFileType, newFilePath, version, isModified}) =>
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
    IntMap.update(id, updater, state);
  /* | BufferDelete(bd) => IntMap.remove(bd, state) */
  | BufferSetModified(id, isModified) =>
    IntMap.update(id, setModified(isModified), state)

  | BufferSetIndentation(id, indent) =>
    IntMap.update(id, setIndentation(indent), state)

  | BufferLineEndingsChanged({id, lineEndings}) =>
    IntMap.update(id, setLineEndings(lineEndings), state)

  | BufferUpdate({update, newBuffer, _}) =>
    IntMap.add(update.id, newBuffer, state)

  | _ => state
  };
};
