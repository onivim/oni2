open TestFramework;

open Oni_Core.Types;
module Buffers = Oni_Model.Buffers;
module Buffer = Oni_Model.Buffer;

let getOrFail = (v: option(Buffer.t)) => {
  let failedMsg = "failed - no buffer was specified";
  switch (v) {
  | Some(v) =>
    let metadata = Buffer.getMetadata(v);
    switch (metadata.filePath) {
    | Some(path) => path
    | None => failedMsg
    };
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
        ),
      );

    expect.string(Buffers.getBuffer(0, added) |> getOrFail).toMatch(
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
        ),
      );
    let addedAgain =
      Buffers.reduce(
        added,
        BufferEnter(
          BufferMetadata.create(~id=0, ~filePath=Some("/test2.re"), ()),
        ),
      );

    expect.string(Buffers.getBuffer(0, addedAgain) |> getOrFail).toMatch(
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
        ),
      );
    let activeBuffer = Buffers.getBuffer(4, updated);
    let path = getOrFail(activeBuffer);
    expect.string(path).toMatch("/myfile.js");
  });
});
