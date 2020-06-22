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
  let keepIndent = (~previousLine as _, ~beforePreviousLine: option(string)) => {
    ignore(beforePreviousLine);
    Vim.AutoIndent.KeepIndent;
  };
  let increaseIndent =
      (~previousLine as _, ~beforePreviousLine: option(string)) => {
    ignore(beforePreviousLine);
    Vim.AutoIndent.IncreaseIndent;
  };
  let decreaseIndent =
      (~previousLine as _, ~beforePreviousLine: option(string)) => {
    ignore(beforePreviousLine);
    Vim.AutoIndent.DecreaseIndent;
  };

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
  test(
    "previous line is set, before previous line is None for first line",
    ({expect, _}) => {
    let _ = resetBuffer();

    let prevRef = ref("");
    let beforePrevRef = ref(Some(""));

    let autoIndent = (~previousLine, ~beforePreviousLine: option(string)) => {
      prevRef := previousLine;
      beforePrevRef := beforePreviousLine;
      AutoIndent.KeepIndent;
    };

    let input = input(~insertSpaces=true, ~autoIndent);
    input("o");

    expect.equal(prevRef^, "This is the first line of a test file");
    expect.equal(beforePrevRef^, None);
  });
  test(
    "previous line is set, before previous line is set for last line",
    ({expect, _}) => {
    let _ = resetBuffer();

    let prevRef = ref("");
    let beforePrevRef = ref(Some(""));

    let autoIndent = (~previousLine, ~beforePreviousLine: option(string)) => {
      prevRef := previousLine;
      beforePrevRef := beforePreviousLine;
      AutoIndent.KeepIndent;
    };

    let input = input(~insertSpaces=true, ~autoIndent);
    // Go to last line
    input("G");
    input("o");

    expect.equal(prevRef^, "This is the third line of a test file");
    expect.equal(
      beforePrevRef^,
      Some("This is the second line of a test file"),
    );
  });

  test("open before line", ({expect, _}) => {
    let _ = resetBuffer();

    let prevRef = ref("");
    let beforePrevRef = ref(Some(""));

    let autoIndent = (~previousLine, ~beforePreviousLine: option(string)) => {
      prevRef := previousLine;
      beforePrevRef := beforePreviousLine;
      AutoIndent.KeepIndent;
    };

    let input = input(~insertSpaces=true, ~autoIndent);
    // Go to last line
    input("j");
    input("O");

    expect.equal(prevRef^, "This is the first line of a test file");
    expect.equal(beforePrevRef^, None);
  });
  test("auto-indent should not be called for first line", ({expect, _}) => {
    let _ = resetBuffer();

    let prevRef = ref("");
    let beforePrevRef = ref(Some(""));
    let autoIndent = (~previousLine, ~beforePreviousLine) => {
      prevRef := previousLine;
      beforePrevRef := beforePreviousLine;
      AutoIndent.KeepIndent;
    };

    let input = input(~insertSpaces=true, ~autoIndent);
    // Open the very first line - auto-indent should not be called in this case
    input("gg");
    input("O");

    expect.equal(prevRef^, "");
    expect.equal(beforePrevRef^, None);
  });
});
