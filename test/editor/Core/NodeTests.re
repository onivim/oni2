open Oni_Core;
open TestFramework;

describe("Node", ({test, _}) => {
  test("version", ({expect}) => {
    let version = Node.version();

    let expectedVersionString = "v10.15.1";

    let actualVersionString =
      String.sub(version, 0, String.length(expectedVersionString));
    expect.string(actualVersionString).toEqual(expectedVersionString);
  });

  test("executeJs", ({expect}) => {
    let v = Node.executeJs("console.log(2+2);"); 

    let expected = "4";

    let actual = String.sub(v, 0, 1);
    expect.string(actual).toEqual(expected);
  });
  });
