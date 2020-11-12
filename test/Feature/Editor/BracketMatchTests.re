open EditorCoreTypes;
open Oni_Core;
open TestFramework;
module LineNumber = EditorCoreTypes.LineNumber;

module BracketMatch = Feature_Editor.BracketMatch;

let create = lines =>
  lines
  |> Array.of_list
  |> Oni_Core.Buffer.ofLines(~font=Font.default())
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
      let result =
        BracketMatch.findFirst(
          ~buffer,
          ~characterPosition=CharacterPosition.zero,
          ~pairs,
        );
      expect.option(result).toBeNone();
    });

    test("pair outside should not get picked up", ({expect, _}) => {
      let buffer = create(["abc", "", "({[a]})"]);
      let characterPosition =
        CharacterPosition.{
          line: LineNumber.(zero + 1),
          character: CharacterIndex.zero,
        };
      let result =
        BracketMatch.findFirst(~buffer, ~characterPosition, ~pairs);
      expect.option(result).toBeNone();
    });

    test("picks nearest", ({expect, _}) => {
      let buffer = create(["({[a]})"]);
      let characterPosition =
        CharacterPosition.{
          line: LineNumber.zero,
          character: CharacterIndex.(zero + 3),
        };
      let result =
        BracketMatch.findFirst(~buffer, ~characterPosition, ~pairs);
      let expected =
        Some(
          BracketMatch.{
            start: {
              line: LineNumber.zero,
              character: CharacterIndex.ofInt(2),
            },
            stop: {
              line: LineNumber.zero,
              character: CharacterIndex.ofInt(4),
            },
          },
        );
      expect.option(result).toBe(expected);
    });

    test("uses closing character if cursor is on it", ({expect, _}) => {
      let buffer = create(["({[a]})"]);
      let characterPosition =
        CharacterPosition.{
          line: LineNumber.zero,
          character: CharacterIndex.(zero + 4),
        };
      let result =
        BracketMatch.findFirst(~buffer, ~characterPosition, ~pairs);
      let expected =
        Some(
          BracketMatch.{
            start: {
              line: LineNumber.zero,
              character: CharacterIndex.ofInt(2),
            },
            stop: {
              line: LineNumber.zero,
              character: CharacterIndex.ofInt(4),
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
      let characterPosition =
        CharacterPosition.{
          line: LineNumber.zero,
          character: CharacterIndex.zero,
        };
      let result =
        BracketMatch.find(
          ~buffer,
          ~characterPosition,
          ~start=Uchar.of_int(0),
          ~stop=Uchar.of_int(0),
        );
      expect.option(result).toBeNone();
    });

    test("out of bounds: line < 0", ({expect, _}) => {
      let buffer = create([""]);
      let characterPosition =
        CharacterPosition.{
          line: LineNumber.ofZeroBased(-1),
          character: CharacterIndex.zero,
        };
      let result =
        BracketMatch.find(
          ~buffer,
          ~characterPosition,
          ~start=Uchar.of_int(0),
          ~stop=Uchar.of_int(0),
        );
      expect.option(result).toBeNone();
    });

    test("out of bounds: line > len0", ({expect, _}) => {
      let buffer = create([""]);
      let characterPosition =
        CharacterPosition.{
          line: LineNumber.ofZeroBased(2),
          character: CharacterIndex.zero,
        };
      let result =
        BracketMatch.find(
          ~buffer,
          ~characterPosition,
          ~start=Uchar.of_int(0),
          ~stop=Uchar.of_int(0),
        );
      expect.option(result).toBeNone();
    });

    test("find pair at start position", ({expect, _}) => {
      let buffer = create(["{}"]);
      let characterPosition = CharacterPosition.zero;
      let result =
        BracketMatch.find(
          ~buffer,
          ~characterPosition,
          ~start=leftBracket,
          ~stop=rightBracket,
        );
      let expected =
        Some(
          BracketMatch.{
            start: {
              line: LineNumber.zero,
              character: CharacterIndex.zero,
            },
            stop: {
              line: LineNumber.zero,
              character: CharacterIndex.ofInt(1),
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
              line: LineNumber.zero,
              character: CharacterIndex.zero,
            },
            stop: {
              line: LineNumber.zero,
              character: CharacterIndex.(zero + 1),
            },
          },
        );
      let characterPosition =
        CharacterPosition.{
          line: LineNumber.zero,
          character: CharacterIndex.(zero + 1),
        };
      let result =
        BracketMatch.find(
          ~buffer,
          ~characterPosition,
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
              line: LineNumber.zero,
              character: CharacterIndex.ofInt(1),
            },
            stop: {
              line: LineNumber.(zero + 2),
              character: CharacterIndex.zero,
            },
          },
        );
      let characterPosition =
        CharacterPosition.{
          line: LineNumber.(zero + 1),
          character: CharacterIndex.(zero + 1),
        };
      let result =
        BracketMatch.find(
          ~buffer,
          ~characterPosition,
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
              line: LineNumber.zero,
              character: CharacterIndex.zero,
            },
            stop: {
              line: LineNumber.(zero + 2),
              character: CharacterIndex.ofInt(3),
            },
          },
        );
      let characterPosition =
        CharacterPosition.{
          line: LineNumber.(zero + 1),
          character: CharacterIndex.zero,
        };
      let result =
        BracketMatch.find(
          ~buffer,
          ~characterPosition,
          ~start=leftBracket,
          ~stop=rightBracket,
        );
      expect.option(result).toBe(expected);
    });
  });
});
