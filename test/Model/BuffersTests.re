open TestFramework;

module Buffers = Feature_Buffers;
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

let emptyBuffer = Oni_Core.Buffer.ofLines([||]);

describe("Buffer List Tests", ({test, _}) => {
  test("Buffer enter should create metadata", ({expect, _}) => {
    let bufferList = Buffers.empty;

    let added =
      Buffers.update(
        Buffers.Entered({
          id: 0,
          buffer: emptyBuffer,
          fileType: None,
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
          fileType: None,
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
          fileType: None,
          lineEndings: None,
          font: Oni_Core.Font.default,
        }),
        added,
      );

    expect.string(Buffers.get(0, addedAgain) |> getFilePathOrFail).
      toMatch(
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
          fileType: None,
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
          fileType: Some("reason"),
          lineEndings: None,
          font: Oni_Core.Font.default,
        }),
        bufferList
      );
    let activeBuffer = Buffers.get(4, updated);
    let fileType = getFileTypeOrFail(activeBuffer);
    expect.string(fileType).toEqual("reason");
  });
});
