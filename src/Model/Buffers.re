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

let applyBufferUpdate = bufferUpdate =>
  Option.map(buffer => Buffer.update(buffer, bufferUpdate));

let setIndentation = indent =>
  Option.map(buffer => Buffer.setIndentation(indent, buffer));

let disableSyntaxHighlighting =
  Option.map(buffer => Buffer.disableSyntaxHighlighting(buffer));

let setModified = modified =>
  Option.map(buffer => Buffer.setModified(modified, buffer));

let reduce = (state: t, action: Actions.t) => {
  switch (action) {
  | BufferDisableSyntaxHighlighting(id) =>
    IntMap.update(id, disableSyntaxHighlighting, state)

  | BufferEnter(metadata, fileType) =>
    let updater = (
      fun
      | Some(buffer) =>
        buffer
        |> Buffer.setModified(metadata.modified)
        |> Buffer.setFilePath(metadata.filePath)
        |> Buffer.setVersion(metadata.version)
        |> Buffer.stampLastUsed
        |> Option.some
      | None =>
        Buffer.ofMetadata(metadata)
        |> Buffer.setFileType(fileType)
        |> Buffer.stampLastUsed
        |> Option.some
    );
    IntMap.update(metadata.id, updater, state);

  /* | BufferDelete(bd) => IntMap.remove(bd, state) */
  | BufferSetModified(id, modified) =>
    IntMap.update(id, setModified(modified), state)

  | BufferSetIndentation(id, indent) =>
    IntMap.update(id, setIndentation(indent), state)

  | BufferUpdate(bu) => IntMap.update(bu.id, applyBufferUpdate(bu), state)

  | BufferSaved(metadata) =>
    IntMap.update(metadata.id, setModified(metadata.modified), state)

  | _ => state
  };
};
