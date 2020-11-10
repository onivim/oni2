open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () => Helpers.resetBuffer("test/testfile.txt");
let input = (~autoClosingPairs=AutoClosingPairs.empty, s) => {
  ignore(
    Vim.input(~context={...Context.current(), autoClosingPairs}, s): (
                                                                    Context.t,
                                                                    list(
                                                                    Effect.t,
                                                                    ),
                                                                    ),
  );
};

let key = (~autoClosingPairs=AutoClosingPairs.empty, s) => {
  ignore(
    Vim.key(~context={...Context.current(), autoClosingPairs}, s): (
                                                                    Context.t,
                                                                    list(
                                                                    Effect.t,
                                                                    ),
                                                                   ),
  );
};

open AutoClosingPairs;
let quote = {|"|};
let parenthesesPair = AutoPair.{opening: "(", closing: ")"};
let squareBracketPair = AutoPair.{opening: "[", closing: "]"};
let curlyBracketPair = AutoPair.{opening: "{", closing: "}"};
let quotePair = AutoPair.{opening: quote, closing: quote};

describe("AutoClosingPairs", ({test, describe, _}) => {
  describe("undo", ({test, _}) => {
    test("single undo undoes entire edit", ({expect, _}) => {
      let b = resetBuffer();

      let autoClosingPairs = AutoClosingPairs.create([squareBracketPair]);
      let lineCount = Buffer.getLineCount(b);
      expect.int(lineCount).toBe(1);

      input(~autoClosingPairs, "o");
      input(~autoClosingPairs, "A");
      input(~autoClosingPairs, "[");
      input(~autoClosingPairs, "]");
      input(~autoClosingPairs, ":");

      let lineCount = Buffer.getLineCount(b);
      expect.int(lineCount).toBe(2);

      // #2635: 'u' should be sufficient to undo the entire line
      key("<ESC>");
      input("u");
      let lineCount = Buffer.getLineCount(b);
      expect.int(lineCount).toBe(1);
    })
  });
  describe("redo", ({test, _}) => {
    test("redo applies entire auto-closing pair", ({expect, _}) => {
      let b = resetBuffer();

      let autoClosingPairs = AutoClosingPairs.create([squareBracketPair]);
      let lineCount = Buffer.getLineCount(b);
      expect.int(lineCount).toBe(1);

      input(~autoClosingPairs, "O");
      input(~autoClosingPairs, "a");
      input(~autoClosingPairs, "[");
      input(~autoClosingPairs, "1");
      input(~autoClosingPairs, "]");
      input(~autoClosingPairs, "b");
      key(~autoClosingPairs, "<esc>");

      let lineCount = Buffer.getLineCount(b);
      expect.int(lineCount).toBe(2);

      let line0Initial = Buffer.getLine(b, LineNumber.zero);
      expect.string(line0Initial).toEqual("a[1]b");

      // #2635: '.' should repeat entire ACP edit
      input(".");
      let line0After = Buffer.getLine(b, LineNumber.zero);
      let line1After = Buffer.getLine(b, LineNumber.(zero + 1));
      expect.string(line0After).toEqual("a[1]b");
      expect.string(line1After).toEqual("a[1]b");
      let lineCount = Buffer.getLineCount(b);
      expect.int(lineCount).toBe(3);
    })
  });
  test("no auto-closing pairs", ({expect, _}) => {
    let b = resetBuffer();

    input("O");
    input("[");
    input("(");
    input("\"");
    input("{");

    let line = Buffer.getLine(b, LineNumber.zero);
    expect.string(line).toEqual("[(\"{");
  });

  test("single acp", ({expect, _}) => {
    let b = resetBuffer();
    let autoClosingPairs = AutoClosingPairs.create([squareBracketPair]);

    input(~autoClosingPairs, "O");
    input(~autoClosingPairs, "[");
    input(~autoClosingPairs, "(");
    input(~autoClosingPairs, "\"");
    input(~autoClosingPairs, "{");

    let line = Buffer.getLine(b, LineNumber.zero);
    expect.string(line).toEqual("[(\"{]");
  });

  describe("enter", ({test, _}) => {
    test(
      "pressing enter between auto-closing pairs should be indented",
      ({expect, _}) => {
      let b = resetBuffer();
      let autoClosingPairs = AutoClosingPairs.create([squareBracketPair]);

      input(~autoClosingPairs, "O");
      input(~autoClosingPairs, "[");
      key(~autoClosingPairs, "<CR>");
      input(~autoClosingPairs, "a");

      let line1 = Buffer.getLine(b, LineNumber.zero);
      let line2 = Buffer.getLine(b, LineNumber.(zero + 1));
      let line3 = Buffer.getLine(b, LineNumber.(zero + 2));
      expect.string(line1).toEqual("[");
      expect.string(line2).toEqual("\ta");
      expect.string(line3).toEqual("]");
    });
    test("indentation should be preserved", ({expect, _}) => {
      let b = resetBuffer();
      let autoClosingPairs = AutoClosingPairs.create([squareBracketPair]);

      input(~autoClosingPairs, "O");
      key(~autoClosingPairs, "<TAB>");
      input(~autoClosingPairs, "[");
      key(~autoClosingPairs, "<CR>");
      input(~autoClosingPairs, "a");

      let line1 = Buffer.getLine(b, LineNumber.zero);
      let line2 = Buffer.getLine(b, LineNumber.(zero + 1));
      let line3 = Buffer.getLine(b, LineNumber.(zero + 2));
      expect.string(line1).toEqual("\t[");
      expect.string(line2).toEqual("\t\ta");
      expect.string(line3).toEqual("\t]");
    });
  });

  test("isBetweenClosingPairs", ({expect, _}) => {
    let b = resetBuffer();
    let autoClosingPairs = AutoClosingPairs.create([squareBracketPair]);

    input(~autoClosingPairs, "O");
    input(~autoClosingPairs, "[");

    let line = Buffer.getLine(b, LineNumber.zero);
    let location = Cursor.get();
    expect.bool(
      AutoClosingPairs.isBetweenClosingPairs(
        line,
        location.byte,
        autoClosingPairs,
      ),
    ).
      toBe(
      true,
    );
  });

  test("backspace between pairs", ({expect, _}) => {
    let b = resetBuffer();

    let autoClosingPairs = AutoClosingPairs.create([squareBracketPair]);

    input(~autoClosingPairs, "O");
    input(~autoClosingPairs, "[");
    key(~autoClosingPairs, "<BS>");

    let line = Buffer.getLine(b, LineNumber.zero);
    expect.string(line).toEqual("");
  });

  test("backspace between pairs, override deletion pairs", ({expect, _}) => {
    let b = resetBuffer();

    let autoClosingPairs =
      AutoClosingPairs.create(
        ~deletionPairs=[squareBracketPair],
        [quotePair],
      );

    input(~autoClosingPairs, "O");
    input(~autoClosingPairs, "[");
    key(~autoClosingPairs, "<BS>");

    let line = Buffer.getLine(b, LineNumber.zero);
    expect.string(line).toEqual("");
  });

  test("pass-through between pairs", ({expect, _}) => {
    let b = resetBuffer();

    let autoClosingPairs = AutoClosingPairs.create([squareBracketPair]);

    input(~autoClosingPairs, "O");
    input(~autoClosingPairs, "[");
    input(~autoClosingPairs, "]");

    let line = Buffer.getLine(b, LineNumber.zero);
    expect.string(line).toEqual("[]");
    expect.int(Cursor.get() |> BytePosition.byte |> ByteIndex.toInt).toBe(2);
  });

  test("pass-through between pairs, overridden", ({expect, _}) => {
    let b = resetBuffer();

    let autoClosingPairs = AutoClosingPairs.create([squareBracketPair]);

    input(~autoClosingPairs, "O");
    input(~autoClosingPairs, "[");

    let autoClosingPairs =
      AutoClosingPairs.create(~passThrough=["]"], [squareBracketPair]);
    input(~autoClosingPairs, "]");

    let line = Buffer.getLine(b, LineNumber.zero);
    expect.string(line).toEqual("[]");
    expect.int(Cursor.get() |> BytePosition.byte |> ByteIndex.toInt).toBe(2);
  });

  test("pass-through not between pairs", ({expect, _}) => {
    let b = resetBuffer();
    let autoClosingPairs = AutoClosingPairs.create([quotePair]);

    input(~autoClosingPairs, "O");
    input(~autoClosingPairs, "(");
    input(~autoClosingPairs, "x");
    input(~autoClosingPairs, ")");

    let line = Buffer.getLine(b, LineNumber.zero);
    expect.string(line).toEqual("(x)");
    expect.int(Cursor.get() |> BytePosition.byte |> ByteIndex.toInt).toBe(3);
  });

  test(
    "pass-through not between pairs, with same begin/end pair", ({expect, _}) => {
    let b = resetBuffer();
    let autoClosingPairs = AutoClosingPairs.create([quotePair]);

    input(~autoClosingPairs, "O");
    input(~autoClosingPairs, quote);
    input(~autoClosingPairs, "x");
    input(~autoClosingPairs, quote);

    let line = Buffer.getLine(b, LineNumber.zero);
    expect.string(line).toEqual({|"x"|});
    expect.int(Cursor.get() |> BytePosition.byte |> ByteIndex.toInt).toBe(3);
  });

  test("can insert closing pair", ({expect, _}) => {
    let b = resetBuffer();
    let autoClosingPairs = AutoClosingPairs.create([quotePair]);

    input(~autoClosingPairs, "O");
    input(~autoClosingPairs, "]");

    let line = Buffer.getLine(b, LineNumber.zero);
    expect.string(line).toEqual("]");
  });

  test("multiple acp", ({expect, _}) => {
    let b = resetBuffer();
    let autoClosingPairs =
      AutoClosingPairs.create(
        ~allowBefore=["]", "}", ")", "\""],
        [squareBracketPair, curlyBracketPair, parenthesesPair, quotePair],
      );

    input(~autoClosingPairs, "O");
    input(~autoClosingPairs, "[");
    input(~autoClosingPairs, "(");
    input(~autoClosingPairs, "\"");
    input(~autoClosingPairs, "{");

    let line = Buffer.getLine(b, LineNumber.zero);
    expect.string(line).toEqual("[(\"{}\")]");
  });
});
