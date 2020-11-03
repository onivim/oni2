open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/testfile.txt");
let input = (~toggleComments, s) =>
  ignore(Vim.input(~context={...Vim.Context.current(), toggleComments}, s));

describe("Comments", ({test, _}) => {
  test("simple toggle case", ({expect, _}) => {
    let buffer = resetBuffer();
    let toggleComments = lines => lines |> Array.map(line => "# " ++ line);

    input(~toggleComments, "g");
    input(~toggleComments, "c");
    input(~toggleComments, "c");

    expect.string(Buffer.getLine(buffer, LineNumber.zero)).toEqual(
      "# This is the first line of a test file",
    );
  })
});
