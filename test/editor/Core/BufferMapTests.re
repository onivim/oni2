open TestFramework;

/* open Helpers; */

open Oni_Core.Types;
module BufferMap = Oni_Core.BufferMap;

describe("Buffer List Tests", ({test, _}) => {
  test(
    "Bufferlist should take a list of buffers and add them to the map",
    ({expect}) => {
    let bufferlist = BufferMap.empty;
    let testBuffers: list(buffer) = [
      {
        filepath: "/test.re",
        id: 0,
        modified: false,
        hidden: false,
        buftype: Empty,
        filetype: "",
      },
      {
        filepath: "/test2.re",
        id: 1,
        modified: false,
        hidden: false,
        buftype: Empty,
        filetype: "",
      },
    ];
    let added = BufferMap.update(bufferlist, testBuffers);
    expect.int(BufferMap.Buffers.cardinal(added)).toBe(2);
  });

  /**
     Neovim buffer IDs should be unique so
     if a different buffer object is received it should replace the buffer

     NOTE: the cardinality of the map is not updated
   */
  test("Bufferlist should override duplicate buffer Ids", ({expect}) => {
    let bufferlist = BufferMap.empty;
    let testBuffers: list(buffer) = [
      {
        filepath: "/test.re",
        id: 1,
        modified: false,
        hidden: false,
        buftype: Empty,
        filetype: "",
      },
      {
        filepath: "/test2.re",
        id: 0,
        modified: false,
        hidden: false,
        buftype: Empty,
        filetype: "",
      },
    ];
    let added = BufferMap.update(bufferlist, testBuffers);

    expect.string(BufferMap.Buffers.find(0, added).filepath).toMatch(
      "/test2.re",
    );
  });

  test("Should correctly find the active bufferId", ({expect}) => {
    let bufferlist = BufferMap.empty;
    let updated =
      BufferMap.Buffers.add(
        4,
        {
          filepath: "/myfile.js",
          id: 4,
          modified: false,
          hidden: false,
          buftype: Empty,
          filetype: "",
        },
        bufferlist,
      );
    let activeBuffer = BufferMap.getActiveBuffer(4, updated);
    expect.string(activeBuffer.filepath).toMatch("/myfile.js");
  });
});
