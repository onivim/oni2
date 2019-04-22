/*
 * Tab.re
 */

open Oni_Core.Types;

type t = {
  id: int,
  title: string,
  active: bool,
  modified: bool,
};

let create = (~id, ~title, ~active=false, ~modified=false, ()) => {
  id,
  title,
  active,
  modified,
};

let ofBuffer = (~buffer: Buffer.t, ~active=false, ()) => {
  let {id, filePath, version, lastSaveVersion, _}: BufferMetadata.t =
    Buffer.getMetadata(buffer);

  let modified = version > lastSaveVersion;

  let title =
    switch (filePath) {
    | Some(v) => v
    | None => "untitled"
    };

  {id, title, active, modified};
};
