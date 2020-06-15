open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/testfile.txt");

let input = (~insertSpaces=false, ~tabSize=3, ~autoIndent, s) => {
  ignore(
    Vim.input(
      ~context={...Context.current(), insertSpaces, tabSize, autoIndent},
      s,
    ): Context.t,
  );
};

describe("AutoIndent", ({test, _}) => {
  let keepIndent = _ => Vim.AutoIndent.KeepIndent;
  let increaseIndent = _ => Vim.AutoIndent.IncreaseIndent;
  let decreaseIndent = _ => Vim.AutoIndent.DecreaseIndent;

  test("keep indent (open line)", ({expect, _}) => {
    let buffer = resetBuffer();
    input(~autoIndent=keepIndent, "o");
    input(~autoIndent=keepIndent, "a");

    let line = Buffer.getLine(buffer, Index.(zero + 1));
    expect.string(line).toEqual("a");
  });

  test("increase indent (open line)", ({expect, _}) => {
    let buffer = resetBuffer();
    input(~autoIndent=increaseIndent, "o");
    input(~autoIndent=increaseIndent, "a");

    let line = Buffer.getLine(buffer, Index.(zero + 1));
    expect.string(line).toEqual("\ta");
  });

  test("increase indent (open line, insert spaces)", ({expect, _}) => {
    let buffer = resetBuffer();

    let input = input(~insertSpaces=true, ~autoIndent=increaseIndent);
    input("o");
    input("a");

    let line = Buffer.getLine(buffer, Index.(zero + 1));
    expect.string(line).toEqual("   a");
  });

  test("decrease indent (open line, insert spaces)", ({expect, _}) => {
    let buffer = resetBuffer();

    let input = input(~insertSpaces=true, ~autoIndent=decreaseIndent);
    input("o");
    input("\t");
    input("a");
    input("<CR>");
    input("b");

    let line = Buffer.getLine(buffer, Index.(zero + 2));
    expect.string(line).toEqual("b");
  });
});
