open TestFramework;

open Vim;

module Buffers = Oni_Model.Buffers;
module Buffer = Oni_Core.Buffer;

let getFilePathOrFail = (v: option(Buffer.t)) => {
  let failedMsg = "failed - no buffer was specified";
  switch (v) {
  | Some(v) =>
    switch (Buffer.getFilePath(v)) {
    | Some(path) => path
    | None => failedMsg
    }
  | None => failedMsg
  };
};

let getFileTypeOrFail = (v: option(Buffer.t)) => {
  let failedMsg = "failed - no buffer was specified";
  switch (v) {
  | Some(v) =>
    switch (Buffer.getFileType(v)) {
    | Some(t) => t
    | None => failedMsg
    }
  | None => failedMsg
  };
};

describe("Buffer List Tests", ({test, _}) => {
  test("Buffer enter should create metadata", ({expect}) => {
    let bufferList = Buffers.empty;

    let added =
      Buffers.reduce(
        bufferList,
        BufferEnter(
          BufferMetadata.create(~id=0, ~filePath=Some("/test1.re"), ()),
          None,
        ),
      );

    expect.string(Buffers.getBuffer(0, added) |> getFilePathOrFail).toMatch(
      "/test1.re",
    );
  });

  /**
     Neovim buffer IDs should be unique so
     if a different buffer object is received it should replace the buffer

     NOTE: the cardinality of the map is not updated
   */
  test("Buffer enter should update metadata, if already there", ({expect}) => {
    let bufferList = Buffers.empty;

    let added =
      Buffers.reduce(
        bufferList,
        BufferEnter(
          BufferMetadata.create(~id=0, ~filePath=Some("/test1.re"), ()),
          None,
        ),
      );
    let addedAgain =
      Buffers.reduce(
        added,
        BufferEnter(
          BufferMetadata.create(~id=0, ~filePath=Some("/test2.re"), ()),
          None,
        ),
      );

    expect.string(Buffers.getBuffer(0, addedAgain) |> getFilePathOrFail).
      toMatch(
      "/test2.re",
    );
  });

  test("Should correctly find the active bufferId", ({expect}) => {
    let bufferList = Buffers.empty;
    let updated =
      Buffers.reduce(
        bufferList,
        BufferEnter(
          BufferMetadata.create(~filePath=Some("/myfile.js"), ~id=4, ()),
          None,
        ),
      );
    let activeBuffer = Buffers.getBuffer(4, updated);
    let path = getFilePathOrFail(activeBuffer);
    expect.string(path).toMatch("/myfile.js");
  });

  test("Should set filetype", ({expect}) => {
    let bufferList = Buffers.empty;
    let updated =
      Buffers.reduce(
        bufferList,
        BufferEnter(
          BufferMetadata.create(~filePath=Some("/myfile.js"), ~id=4, ()),
          Some("reason"),
        ),
      );
    let activeBuffer = Buffers.getBuffer(4, updated);
    let fileType = getFileTypeOrFail(activeBuffer);
    expect.string(fileType).toEqual("reason");
  });
});
