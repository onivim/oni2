open TestFramework;

/* open Helpers; */

open Oni_Core.Types;
module BufferMap = Oni_Core.BufferMap;
module Buffer = Oni_Core.Buffer;

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
  test(
    "Bufferlist should take a list of buffers and add them to the map",
    ({expect}) => {
    let bufferlist = BufferMap.empty;
    let testBuffers: list(BufferMetadata.t) = [
      BufferMetadata.create(~id=1, ~filePath=Some("/test.re"), ()),
      BufferMetadata.create(~id=0, ~filePath=Some("/test2.re"), ()),
    ];
    let added = BufferMap.updateMetadata(bufferlist, testBuffers);
    expect.int(BufferMap.Buffers.cardinal(added)).toBe(2);
  });

  /**
     Neovim buffer IDs should be unique so
     if a different buffer object is received it should replace the buffer

     NOTE: the cardinality of the map is not updated
   */
  test("Bufferlist should override duplicate buffer Ids", ({expect}) => {
    let bufferlist = BufferMap.empty;
    let testBuffers: list(BufferMetadata.t) = [
      BufferMetadata.create(~id=1, ~filePath=Some("/test.re"), ()),
      BufferMetadata.create(~id=0, ~filePath=Some("/test2.re"), ()),
    ];
    let added = BufferMap.updateMetadata(bufferlist, testBuffers);

    expect.string(BufferMap.getBuffer(0, added) |> getOrFail).toMatch(
      "/test2.re",
    );
  });

  test("Should correctly find the active bufferId", ({expect}) => {
    let bufferlist = BufferMap.empty;
    let updated =
      BufferMap.Buffers.add(
        4,
        Buffer.ofMetadata({
          filePath: Some("/myfile.js"),
          id: 4,
          modified: false,
          hidden: false,
          bufType: Empty,
          fileType: None,
          version: 0,
        }),
        bufferlist,
      );
    let activeBuffer = BufferMap.getBuffer(4, updated);
    let path = getOrFail(activeBuffer);
    expect.string(path).toMatch("/myfile.js");
  });
});
