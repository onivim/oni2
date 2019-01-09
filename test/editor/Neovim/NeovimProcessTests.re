open Rench;

open Oni_Neovim;
open TestFramework;

open Helpers;

describe("NeovimProcess", ({test, _}) =>
  test("version", ({expect}) => {
    let neovimPath = Helpers.getNeovimPath();

    let version = NeovimProcess.version(~neovimPath);

    let expectedVersionString = "NVIM v0.3.3";

    let actualVersionString =
      String.sub(version, 0, String.length(expectedVersionString));
    expect.string(actualVersionString).toEqual(expectedVersionString);
  })
);
