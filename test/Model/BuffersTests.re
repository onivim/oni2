open Oni_Core.Utility;
open TestFramework;

module Buffers = Feature_Buffers;
module Buffer = Oni_Core.Buffer;

let getFilePathOrFail = (maybeBuffer: option(Buffer.t)) => {
  maybeBuffer |> OptionEx.flatMap(Buffer.getFilePath) |> Option.get;
};

let getFileTypeOrFail = (maybeBuffer: option(Buffer.t)) => {
  maybeBuffer
  |> Option.map(Buffer.getFileType)
  |> Option.map(Buffer.FileType.toString)
  |> Option.get;
};

let emptyBuffer = Oni_Core.Buffer.ofLines([||]);

describe("Buffer List Tests", ({test, _}) => {
  test("Buffer enter should create metadata", ({expect, _}) => {
    let bufferList = Buffers.empty;

    let added =
      Buffers.update(
        Buffers.Entered({
          id: 0,
          buffer: emptyBuffer,
          fileType: Buffer.FileType.none,
          lineEndings: None,
          isModified: false,
          version: 0,
          filePath: Some("/test1.re"),
          font: Oni_Core.Font.default,
        }),
        bufferList,
      );

    expect.string(Buffers.get(0, added) |> getFilePathOrFail).toMatch(
      "/test1.re",
    );
  });

  /**
     Neovim buffer IDs should be unique so
     if a different buffer object is received it should replace the buffer

     NOTE: the cardinality of the map is not updated
   */
  test(
    "Buffer enter should update metadata, if already there", ({expect, _}) => {
    let bufferList = Buffers.empty;

    let added =
      Buffers.update(
        Buffers.Entered({
          id: 0,
          buffer: emptyBuffer,
          filePath: Some("/test1.re"),
          isModified: false,
          version: 0,
          fileType: Buffer.FileType.none,
          lineEndings: None,
          font: Oni_Core.Font.default,
        }),
        bufferList,
      );
    let addedAgain =
      Buffers.update(
        Buffers.Entered({
          id: 0,
          buffer: emptyBuffer,
          filePath: Some("/test2.re"),
          isModified: false,
          version: 0,
          fileType: Buffer.FileType.none,
          lineEndings: None,
          font: Oni_Core.Font.default,
        }),
        added,
      );

    expect.string(Buffers.get(0, addedAgain) |> getFilePathOrFail).toMatch(
      "/test2.re",
    );
  });

  test("Should correctly find the active bufferId", ({expect, _}) => {
    let bufferList = Buffers.empty;
    let updated =
      Buffers.update(
        Buffers.Entered({
          id: 4,
          buffer: emptyBuffer,
          filePath: Some("/myfile.js"),
          isModified: false,
          version: 0,
          fileType: Buffer.FileType.none,
          lineEndings: None,
          font: Oni_Core.Font.default,
        }),
        bufferList,
      );
    let activeBuffer = Buffers.get(4, updated);
    let path = getFilePathOrFail(activeBuffer);
    expect.string(path).toMatch("/myfile.js");
  });

  test("Should set filetype", ({expect, _}) => {
    let bufferList = Buffers.empty;
    let updated =
      Buffers.update(
        Buffers.Entered({
          id: 4,
          buffer: emptyBuffer,
          filePath: Some("/myfile.js"),
          isModified: false,
          version: 0,
          fileType: Buffer.FileType.inferred("reason"),
          lineEndings: None,
          font: Oni_Core.Font.default,
        }),
        bufferList,
      );
    let activeBuffer = Buffers.get(4, updated);
    let fileType = getFileTypeOrFail(activeBuffer);
    expect.string(fileType).toEqual("reason");
  });
});
