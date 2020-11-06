open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/testfile.txt");
let input = s => ignore(Vim.input(s));
let key = s => ignore(Vim.key(s));

describe("ReplaceEdit", ({test, _}) => {
  test("simple replace", ({expect, _}) => {
    let buffer = resetBuffer();

    input("r");
    input("a");

    expect.bool(Mode.isNormal(Mode.current())).toBe(true);
    expect.string(Buffer.getLine(buffer, LineNumber.zero)).toEqual(
      "ahis is the first line of a test file",
    );
  });
  test("#2644 - escape key should cancel replace", ({expect, _}) => {
    let buffer = resetBuffer();

    input("r");
    key("<esc>");

    expect.bool(Mode.isNormal(Mode.current())).toBe(true);
    expect.string(Buffer.getLine(buffer, LineNumber.zero)).toEqual(
      "This is the first line of a test file",
    );
  });
});
