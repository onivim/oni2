open Oni_Core;
open TestFramework;

module BracketMatch = Feature_Editor.BracketMatch;

let create = lines =>
  lines
  |> Array.of_list
  |> Oni_Core.Buffer.ofLines
  |> Feature_Editor.EditorBuffer.ofBuffer;

let pairs =
  LanguageConfiguration.BracketPair.[
    {openPair: "{", closePair: "}"},
    {openPair: "(", closePair: ")"},
    {openPair: "[", closePair: "]"},
  ];

describe("BracketMatch", ({describe, _}) => {
  describe("findFirst", ({test, _}) => {
    test("empty buffer", ({expect, _}) => {
      let buffer = create([""]);
      let result = BracketMatch.findFirst(~buffer, ~line=0, ~index=0, ~pairs);
      expect.option(result).toBeNone();
    });

    test("pair outside should not get picked up", ({expect, _}) => {
      let buffer = create(["abc", "", "({[a]})"]);
      let result = BracketMatch.findFirst(~buffer, ~line=1, ~index=0, ~pairs);
      expect.option(result).toBeNone();
    });

    test("picks nearest", ({expect, _}) => {
      let buffer = create(["({[a]})"]);
      let result = BracketMatch.findFirst(~buffer, ~line=0, ~index=3, ~pairs);
      let expected =
        Some(
          BracketMatch.{
            start: {
              line: 0,
              index: 2,
            },
            stop: {
              line: 0,
              index: 4,
            },
          },
        );
      expect.option(result).toBe(expected);
    });

    test("uses closing character if cursor is on it", ({expect, _}) => {
      let buffer = create(["({[a]})"]);
      let result = BracketMatch.findFirst(~buffer, ~line=0, ~index=4, ~pairs);
      let expected =
        Some(
          BracketMatch.{
            start: {
              line: 0,
              index: 2,
            },
            stop: {
              line: 0,
              index: 4,
            },
          },
        );
      expect.option(result).toBe(expected);
    });
  });

  describe("find", ({test, _}) => {
    let leftBracket = Uchar.of_char('{');
    let rightBracket = Uchar.of_char('}');

    test("empty buffer", ({expect, _}) => {
      let buffer = create([""]);
      let result =
        BracketMatch.find(
          ~buffer,
          ~line=0,
          ~index=0,
          ~start=Uchar.of_int(0),
          ~stop=Uchar.of_int(0),
        );
      expect.option(result).toBeNone();
    });

    test("out of bounds: line < 0", ({expect, _}) => {
      let buffer = create([""]);
      let result =
        BracketMatch.find(
          ~buffer,
          ~line=-1,
          ~index=0,
          ~start=Uchar.of_int(0),
          ~stop=Uchar.of_int(0),
        );
      expect.option(result).toBeNone();
    });

    test("out of bounds: line > len0", ({expect, _}) => {
      let buffer = create([""]);
      let result =
        BracketMatch.find(
          ~buffer,
          ~line=2,
          ~index=0,
          ~start=Uchar.of_int(0),
          ~stop=Uchar.of_int(0),
        );
      expect.option(result).toBeNone();
    });

    test("find pair at start position", ({expect, _}) => {
      let buffer = create(["{}"]);
      let result =
        BracketMatch.find(
          ~buffer,
          ~line=0,
          ~index=0,
          ~start=leftBracket,
          ~stop=rightBracket,
        );
      let expected =
        Some(
          BracketMatch.{
            start: {
              line: 0,
              index: 0,
            },
            stop: {
              line: 0,
              index: 1,
            },
          },
        );
      expect.option(result).toBe(expected);
    });

    test("find pair before start position", ({expect, _}) => {
      let buffer = create(["{}"]);

      let expected =
        Some(
          BracketMatch.{
            start: {
              line: 0,
              index: 0,
            },
            stop: {
              line: 0,
              index: 1,
            },
          },
        );
      let result =
        BracketMatch.find(
          ~buffer,
          ~line=0,
          ~index=1,
          ~start=leftBracket,
          ~stop=rightBracket,
        );
      expect.option(result).toBe(expected);
    });

    test("find before/after line", ({expect, _}) => {
      let buffer = create(["a{", "bc", "}d"]);

      let expected =
        Some(
          BracketMatch.{
            start: {
              line: 0,
              index: 1,
            },
            stop: {
              line: 2,
              index: 0,
            },
          },
        );
      let result =
        BracketMatch.find(
          ~buffer,
          ~line=1,
          ~index=1,
          ~start=leftBracket,
          ~stop=rightBracket,
        );
      expect.option(result).toBe(expected);
    });

    test("skip nested", ({expect, _}) => {
      let buffer = create(["{a{}", "bc", "{}d}"]);
      let expected =
        Some(
          BracketMatch.{
            start: {
              line: 0,
              index: 0,
            },
            stop: {
              line: 2,
              index: 3,
            },
          },
        );
      let result =
        BracketMatch.find(
          ~buffer,
          ~line=1,
          ~index=0,
          ~start=leftBracket,
          ~stop=rightBracket,
        );
      expect.option(result).toBe(expected);
    });
  });
});
