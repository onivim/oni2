open Oni_Core;
open TestFramework;

describe("Node", ({test, _}) =>
  test("version", ({expect}) => {
    let version = Node.version();

    let expectedVersionString = "v10.15.1";

    let actualVersionString =
      String.sub(version, 0, String.length(expectedVersionString));
    expect.string(actualVersionString).toEqual(expectedVersionString);
  })
);
