open Oni_Neovim;
open TestFramework;

describe("NeovimProcess", ({test, _}) =>
  test("version", ({expect}) => {
    let version = NeovimProcess.version();

    let expectedVersionString = "NVIM v0.3.3";

    let actualVersionString =
      String.sub(version, 0, String.length(expectedVersionString));
    expect.string(actualVersionString).toEqual(expectedVersionString);
  })
);
