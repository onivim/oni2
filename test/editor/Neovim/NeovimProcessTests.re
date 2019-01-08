open Rench;

open Oni_Neovim;
open TestFramework;

/* open Helpers; */

exception EnvironmentVariableNotFound;

let optOrThrow = (s: option(string)) => {
  switch (s) {
  | Some(v) => v
  | _ => raise(EnvironmentVariableNotFound)
  };
};

describe("NeovimProcess", ({test, _}) =>
  test("version", ({expect}) => {
    let neovimPath =
      Environment.getEnvironmentVariable("ONI2_NEOVIM_PATH") |> optOrThrow;

    let version = NeovimProcess.version(~neovimPath);

    let expectedVersionString = "NVIM v0.3.3";

    let actualVersionString =
      String.sub(version, 0, String.length(expectedVersionString));
    expect.string(actualVersionString).toEqual(expectedVersionString);
  })
);
