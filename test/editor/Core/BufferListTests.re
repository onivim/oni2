open TestFramework;

/* open Helpers; */

open Oni_Core.Types;
module BuffersMap = Oni_Core.BuffersMap;

describe("Buffer List Tests", ({ test, _ }) => {
  test("Bufferlist should take a list of buffers and add them to the map", ({ expect }) => {
    let bufferlist = BuffersMap.empty;
    let testBuffers: list(buffer) = [{ filepath: "/test.re", id: 0 }, { filepath: "/test2.re", id: 1 }]
    let added = BuffersMap.update(bufferlist, testBuffers);
    expect.int(BuffersMap.Buffers.cardinal(added)).toBe(2);
  })

  /**
     Neovim buffer IDs should be unique so 
     if a different buffer object is received it should replace the buffer

     NOTE: the cardinality of the map is not updated
   */
  test("Bufferlist should override duplicate buffer Ids", ({ expect }) => {
    let bufferlist = BuffersMap.empty;
    let testBuffers: list(buffer) = [{ filepath: "/test.re", id: 1 }, { filepath: "/test2.re", id: 0 }];
    let added = BuffersMap.update(bufferlist, testBuffers);

    expect.string(BuffersMap.Buffers.find(0, added).filepath).toMatch("/test2.re");
  })

  test("Should correctly find the active bufferId", ({ expect }) => {
          let bufferlist = BuffersMap.empty;
          let updated = BuffersMap.Buffers.add(4, { filepath: "/myfile.js", id: 4 }, bufferlist);
          let activeBuffer = BuffersMap.getActiveBuffer(4, updated);
          expect.string(activeBuffer.filepath).toMatch("/myfile.js");
      })
})
