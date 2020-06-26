open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/testfile.txt");

let resetBufferIndent2Spaces = () =>
  Helpers.resetBuffer("test/reason-libvim/indent-two-spaces.txt");

describe("AutoIndent", ({describe, _}) => {
  describe("onTypeAutoIndent", ({test, _}) => {
    let decreaseIndentWithCharacter = (c, str) => {
      String.index_opt(str, c) == None
        ? Vim.AutoIndent.KeepIndent : Vim.AutoIndent.DecreaseIndent;
    };
    let input = (~insertSpaces=true, ~tabSize=3, ~onTypeAutoIndent, s) => {
      ignore(
        Vim.input(
          ~context={
            ...Context.current(),
            insertSpaces,
            tabSize,
            onTypeAutoIndent,
          },
          s,
        ): Context.t,
      );
    };

    test("decrease indent on type, conditionally", ({expect, _}) => {
      let buffer = resetBuffer();
      let input = input(~onTypeAutoIndent=decreaseIndentWithCharacter('}'));

      input("O");
      input("   a");

      let line = Buffer.getLine(buffer, Index.(zero));
      expect.string(line).toEqual("   a");

      input("}");
      let line = Buffer.getLine(buffer, Index.(zero));
      expect.string(line).toEqual("a}");
    });
  });
  describe("onOpenAutoIndent", ({test, _}) => {
    let input = (~insertSpaces=false, ~tabSize=3, ~autoIndent, s) => {
      ignore(
        Vim.input(
          ~context={
            ...Context.current(),
            insertSpaces,
            tabSize,
            onOpenAutoIndent: autoIndent,
          },
          s,
        ): Context.t,
      );
    };
    let keepIndent =
        (~previousLine as _, ~beforePreviousLine: option(string)) => {
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
    
    test(
      "open before indented line, after empty line, keep indent", ({expect, _}) => {
      let buffer = resetBufferIndent2Spaces();

      buffer
      |> Vim.Buffer.setLines(
           ~lines=[|
             "line 1",
             "", // Add a line with spaces
             "  line2",
             "    line3",
           |],
         );

      let input = input(~insertSpaces=true, ~autoIndent=keepIndent);
      // Go to third line
      input("gg");
      input("j");
      input("j");
      input("O");
      input("a");

      let line = Buffer.getLine(buffer, Index.(zero + 2));
      expect.string(line).toEqual("  a");
    });

    test("open before indented line, keep indent", ({expect, _}) => {
      let buffer = resetBufferIndent2Spaces();

      let input = input(~insertSpaces=true, ~autoIndent=keepIndent);
      // Go to second line
      input("j");
      input("O");
      input("a");

      let line = Buffer.getLine(buffer, Index.(zero + 1));
      expect.string(line).toEqual("a");
    });

    test("open before indented line, indent", ({expect, _}) => {
      let buffer = resetBufferIndent2Spaces();

      let input = input(~insertSpaces=true, ~autoIndent=increaseIndent);
      // Go to second line
      input("j");
      input("O");
      input("a");

      let line = Buffer.getLine(buffer, Index.(zero + 1));
      expect.string(line).toEqual("  a");
    });

    test("open before indented line, un-indent", ({expect, _}) => {
      let buffer = resetBufferIndent2Spaces();

      let input = input(~insertSpaces=true, ~autoIndent=decreaseIndent);
      // Go to second line
      input("j");
      input("j");
      input("O");
      input("a");

      let line = Buffer.getLine(buffer, Index.(zero + 2));
      expect.string(line).toEqual("a");
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
});
