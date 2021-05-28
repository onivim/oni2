open EditorCoreTypes;
open TestFramework;
open Oni_Core;
open Feature_LanguageSupport;
module LineNumber = EditorCoreTypes.LineNumber;

let makeLine = str => BufferLine.make(~measure=_ => 1.0, str);

describe("CompletionMeet", ({describe, _}) => {
  describe("createFromLine", ({test, _}) => {
    let line0column0 =
      CharacterPosition.{
        line: LineNumber.zero,
        character: CharacterIndex.zero,
      };
    let line0column1 =
      CharacterPosition.{
        line: LineNumber.zero,
        character: CharacterIndex.(zero + 1),
      };
    let line0column2 =
      CharacterPosition.{
        line: LineNumber.zero,
        character: CharacterIndex.(zero + 2),
      };
    let line0column8 =
      CharacterPosition.{
        line: LineNumber.zero,
        character: CharacterIndex.(zero + 8),
      };

    test("empty line - no meet", ({expect, _}) => {
      let result =
        CompletionMeet.fromLine(
          ~languageConfiguration=LanguageConfiguration.default,
          ~index=CharacterIndex.(zero + 1),
          ~bufferId=0,
          "" |> makeLine,
        );
      expect.equal(result, None);
    });

    test("single character at beginning", ({expect, _}) => {
      let result =
        CompletionMeet.fromLine(
          ~languageConfiguration=LanguageConfiguration.default,
          ~index=CharacterIndex.(zero + 1),
          ~bufferId=0,
          "a" |> makeLine,
        );

      let expected =
        CompletionMeet.{
          location: line0column1,
          insertLocation: line0column0,
          base: "a",
          bufferId: 0,
        };

      expect.equal(result, Some(expected));
    });

    test("spaces prior to character", ({expect, _}) => {
      let result =
        CompletionMeet.fromLine(
          ~languageConfiguration=LanguageConfiguration.default,
          ~index=CharacterIndex.(zero + 2),
          ~bufferId=0,
          " a" |> makeLine,
        );

      let expected =
        CompletionMeet.{
          location: line0column2,
          insertLocation: line0column1,
          base: "a",
          bufferId: 0,
        };
      expect.equal(result, Some(expected));
    });

    test("between non-word characters", ({expect, _}) => {
      let result =
        CompletionMeet.fromLine(
          ~languageConfiguration=LanguageConfiguration.default,
          ~index=CharacterIndex.(zero + 3),
          ~bufferId=0,
          " (a)" |> makeLine,
        );

      let expected =
        CompletionMeet.{
          location: line0column2,
          insertLocation: line0column2,
          base: "a",
          bufferId: 0,
        };
      expect.equal(result, Some(expected));
    });

    test("longer base", ({expect, _}) => {
      let result =
        CompletionMeet.fromLine(
          ~languageConfiguration=LanguageConfiguration.default,
          ~index=CharacterIndex.(zero + 4),
          ~bufferId=0,
          " abc" |> makeLine,
        );
      let expected =
        CompletionMeet.{
          location: line0column2,
          insertLocation: line0column1,
          base: "abc",
          bufferId: 0,
        };
      expect.equal(result, Some(expected));
    });

    test("default trigger character", ({expect, _}) => {
      let result =
        CompletionMeet.fromLine(
          ~languageConfiguration=LanguageConfiguration.default,
          ~index=CharacterIndex.(zero + 2),
          ~bufferId=0,
          " ." |> makeLine,
        );
      let expected =
        CompletionMeet.{
          location: line0column2,
          insertLocation: line0column2,
          base: "",
          bufferId: 0,
        };
      expect.equal(result, Some(expected));
    });

    test("default trigger character with base", ({expect, _}) => {
      let result =
        CompletionMeet.fromLine(
          ~languageConfiguration=LanguageConfiguration.default,
          ~index=CharacterIndex.(zero + 10),
          ~bufferId=0,
          "console.lo" |> makeLine,
        );
      let expected =
        CompletionMeet.{
          location: line0column8,
          insertLocation: line0column8,
          base: "lo",
          bufferId: 0,
        };
      expect.equal(result, Some(expected));
    });
  })
});
