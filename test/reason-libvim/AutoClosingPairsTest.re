open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () => Helpers.resetBuffer("test/testfile.txt");
let input = (~autoClosingPairs=AutoClosingPairs.empty, s) => {
  ignore(
    Vim.input(~context={...Context.current(), autoClosingPairs}, s): Context.t,
  );
};

open AutoClosingPairs;
let quote = {|"|};
let parenthesesPair = AutoPair.{opening: "(", closing: ")"};
let squareBracketPair = AutoPair.{opening: "[", closing: "]"};
let curlyBracketPair = AutoPair.{opening: "{", closing: "}"};
let quotePair = AutoPair.{opening: quote, closing: quote};

describe("AutoClosingPairs", ({test, describe, _}) => {
  test("no auto-closing pairs", ({expect, _}) => {
    let b = resetBuffer();

    input("O");
    input("[");
    input("(");
    input("\"");
    input("{");

    let line = Buffer.getLine(b, Index.zero);
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

    let line = Buffer.getLine(b, Index.zero);
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
      input(~autoClosingPairs, "<CR>");
      input(~autoClosingPairs, "a");

      let line1 = Buffer.getLine(b, Index.zero);
      let line2 = Buffer.getLine(b, Index.(zero + 1));
      let line3 = Buffer.getLine(b, Index.(zero + 2));
      expect.string(line1).toEqual("[");
      expect.string(line2).toEqual("\ta");
      expect.string(line3).toEqual("]");
    });
    test("indentation should be preserved", ({expect, _}) => {
      let b = resetBuffer();
      let autoClosingPairs = AutoClosingPairs.create([squareBracketPair]);

      input(~autoClosingPairs, "O");
      input(~autoClosingPairs, "<TAB>");
      input(~autoClosingPairs, "[");
      input(~autoClosingPairs, "<CR>");
      input(~autoClosingPairs, "a");

      let line1 = Buffer.getLine(b, Index.zero);
      let line2 = Buffer.getLine(b, Index.(zero + 1));
      let line3 = Buffer.getLine(b, Index.(zero + 2));
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

    let line = Buffer.getLine(b, Index.zero);
    let location = Cursor.getLocation();
    expect.bool(
      AutoClosingPairs.isBetweenClosingPairs(
        line,
        location.column,
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
    input(~autoClosingPairs, "<BS>");

    let line = Buffer.getLine(b, Index.zero);
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
    input(~autoClosingPairs, "<BS>");

    let line = Buffer.getLine(b, Index.zero);
    expect.string(line).toEqual("");
  });

  test("pass-through between pairs", ({expect, _}) => {
    let b = resetBuffer();

    let autoClosingPairs = AutoClosingPairs.create([squareBracketPair]);

    input(~autoClosingPairs, "O");
    input(~autoClosingPairs, "[");
    input(~autoClosingPairs, "]");

    let line = Buffer.getLine(b, Index.zero);
    let location = Cursor.getLocation();
    expect.string(line).toEqual("[]");
    expect.int((location.column :> int)).toBe(2);
  });

  test("pass-through between pairs, overridden", ({expect, _}) => {
    let b = resetBuffer();

    let autoClosingPairs = AutoClosingPairs.create([squareBracketPair]);

    input(~autoClosingPairs, "O");
    input(~autoClosingPairs, "[");

    let autoClosingPairs =
      AutoClosingPairs.create(~passThrough=["]"], [squareBracketPair]);
    input(~autoClosingPairs, "]");

    let line = Buffer.getLine(b, Index.zero);
    let location = Cursor.getLocation();
    expect.string(line).toEqual("[]");
    expect.int((location.column :> int)).toBe(2);
  });

  test("pass-through not between pairs", ({expect, _}) => {
    let b = resetBuffer();
    let autoClosingPairs = AutoClosingPairs.create([quotePair]);

    input(~autoClosingPairs, "O");
    input(~autoClosingPairs, "(");
    input(~autoClosingPairs, "x");
    input(~autoClosingPairs, ")");

    let line = Buffer.getLine(b, Index.zero);
    let location = Cursor.getLocation();
    expect.string(line).toEqual("(x)");
    expect.int((location.column :> int)).toBe(3);
  });

  test(
    "pass-through not between pairs, with same begin/end pair", ({expect, _}) => {
    let b = resetBuffer();
    let autoClosingPairs = AutoClosingPairs.create([quotePair]);

    input(~autoClosingPairs, "O");
    input(~autoClosingPairs, quote);
    input(~autoClosingPairs, "x");
    input(~autoClosingPairs, quote);

    let line = Buffer.getLine(b, Index.zero);
    let location = Cursor.getLocation();
    expect.string(line).toEqual({|"x"|});
    expect.int((location.column :> int)).toBe(3);
  });

  test("can insert closing pair", ({expect, _}) => {
    let b = resetBuffer();
    let autoClosingPairs = AutoClosingPairs.create([quotePair]);

    input(~autoClosingPairs, "O");
    input(~autoClosingPairs, "]");

    let line = Buffer.getLine(b, Index.zero);
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

    let line = Buffer.getLine(b, Index.zero);
    expect.string(line).toEqual("[(\"{}\")]");
  });
});
