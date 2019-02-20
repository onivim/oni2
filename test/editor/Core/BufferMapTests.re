open TestFramework;

/* open Helpers; */

open Oni_Core.Types;
module BufferMap = Oni_Core.BufferMap;
module Buffer = Oni_Core.Buffer;

let getOrFail = (v: option(string)) => switch(v) {
| Some(v) => v
| None => "failed - no buffer was specified"
};

describe("Buffer List Tests", ({test, _}) => {
  test(
    "Bufferlist should take a list of buffers and add them to the map",
    ({expect}) => {
    let bufferlist = BufferMap.empty;
    let testBuffers: list(BufferMetadata.t) = [
      {
        filePath: Some("/test.re"),
        id: 0,
        modified: false,
        hidden: false,
        bufType: Empty,
        fileType: None,
      },
      {
        filePath: Some("/test2.re"),
        id: 1,
        modified: false,
        hidden: false,
        bufType: Empty,
        fileType: None,
      },
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
      {
        filePath: Some("/test.re"),
        id: 1,
        modified: false,
        hidden: false,
        bufType: Empty,
        fileType: None,
      },
      {
        filePath: Some("/test2.re"),
        id: 0,
        modified: false,
        hidden: false,
        bufType: Empty,
        fileType: None,
      },
    ];
    let added = BufferMap.updateMetadata(bufferlist, testBuffers);

    expect.string(BufferMap.Buffers.find(0, added).metadata.filePath |> getOrFail).toMatch(
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
        }),
        bufferlist,
      );
    let activeBuffer = BufferMap.getBuffer(4, updated);
    expect.string(activeBuffer.metadata.filePath |> getOrFail).toMatch("/myfile.js");
  });
});
