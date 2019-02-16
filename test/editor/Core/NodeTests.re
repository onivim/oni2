open Oni_Core;
open TestFramework;

describe("Node", ({test, _}) => {
  test("version", ({expect}) => {
    let {nodePath, _}: Setup.t = Setup.init();
    let version = Node.version(~nodePath, ());

    let expectedVersionString = "v10.15.1";

    let actualVersionString =
      String.sub(version, 0, String.length(expectedVersionString));
    expect.string(actualVersionString).toEqual(expectedVersionString);
  });

  test("executeJs", ({expect}) => {
    let {nodePath, _}: Setup.t = Setup.init();
    let v = Node.executeJs(~nodePath, "console.log(2+2);"); 

    let expected = "4";

    let actual = String.sub(v, 0, 1);
    expect.string(actual).toEqual(expected);
  });
  });
