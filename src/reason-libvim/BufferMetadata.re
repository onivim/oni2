type t = {
  filePath: option(string),
  fileType: option(string),
  modified: bool,
  id: int,
  version: int,
};

let ofBuffer = (buffer: Native.buffer) => {
  id: Native.vimBufferGetId(buffer),
  filePath: Native.vimBufferGetFilename(buffer),
  fileType: Native.vimBufferGetFiletype(buffer),
  modified: Native.vimBufferGetModified(buffer),
  version: Native.vimBufferGetChangedTick(buffer),
};

let create =
    (~filePath=None, ~fileType=None, ~modified=false, ~id=0, ~version=0, ()) => {
  filePath,
  fileType,
  modified,
  id,
  version,
};

let pp = (_: t) => "";
