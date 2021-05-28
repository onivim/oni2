open Oni_Core;
open TestFramework;

open Exthost;

describe("ModelContentChange", ({describe, _}) => {
  describe("ofMinimalUpdates", ({describe, _}) => {
    let getChanges = (previousLines, newLines) => {
      let font = Oni_Core.Font.default();
      let previousBuffer =
        previousLines |> Array.of_list |> Oni_Core.Buffer.ofLines(~font);

      let newBuffer =
        newLines |> Array.of_list |> Oni_Core.Buffer.ofLines(~font);

      let minimalUpdate =
        MinimalUpdate.fromBuffers(
          ~original=previousBuffer,
          ~updated=newBuffer,
        );

      ModelContentChange.ofMinimalUpdates(
        ~previousBuffer,
        ~eol=Exthost.Eol.LF,
        minimalUpdate,
      );
    };

    let range = (startLineNumber, startColumn, endLineNumber, endColumn) => {
      OneBasedRange.{startLineNumber, startColumn, endLineNumber, endColumn};
    };

    describe("adding lines", ({test, _}) => {
      test("insert into empty buffer", ({expect, _}) => {
        let before = [""];
        let after = ["abc"];
        let actualChanges = getChanges(before, after);

        let expectedChanges =
          ModelContentChange.[
            {range: range(1, 1, 1, 1), text: "abc", rangeLength: 0},
          ];

        expect.equal(expectedChanges, actualChanges);
      });

      test("insert + newline into empty buffer", ({expect, _}) => {
        let before = [""];
        let after = ["abc", ""];
        let actualChanges = getChanges(before, after);

        let expectedChanges =
          ModelContentChange.[
            {range: range(1, 1, 1, 1), text: "abc\n", rangeLength: 0},
          ];

        expect.equal(expectedChanges, actualChanges);
      });

      test("insert line before existing buffer text", ({expect, _}) => {
        let before = ["abc"];
        let after = ["def", "abc"];
        let actualChanges = getChanges(before, after);

        let expectedChanges =
          ModelContentChange.[
            {range: range(1, 1, 1, 1), text: "def\n", rangeLength: 0},
          ];

        expect.equal(expectedChanges, actualChanges);
      });

      test("insert line after existing buffer text", ({expect, _}) => {
        let before = ["abc"];
        let after = ["abc", "def"];
        let actualChanges = getChanges(before, after);

        let expectedChanges =
          ModelContentChange.[
            {range: range(1, 4, 1, 4), text: "\ndef", rangeLength: 0},
          ];

        expect.equal(expectedChanges, actualChanges);
      });

      test("insert line after existing buffer text (utf8)", ({expect, _}) => {
        let before = ["κόσμε"];
        let after = ["κόσμε", "def"];
        let actualChanges = getChanges(before, after);

        let expectedChanges =
          ModelContentChange.[
            {range: range(1, 6, 1, 6), text: "\ndef", rangeLength: 0},
          ];

        expect.equal(expectedChanges, actualChanges);
      });
    });

    describe("modification", ({test, _}) => {
      test("modify existing line", ({expect, _}) => {
        let before = ["abc"];
        let after = ["abcd"];
        let actualChanges = getChanges(before, after);

        let expectedChanges =
          ModelContentChange.[
            {range: range(1, 1, 1, 4), text: "abcd", rangeLength: 3},
          ];

        expect.equal(expectedChanges, actualChanges);
      });

      test("modify existing line (utf-8)", ({expect, _}) => {
        let before = ["κόσμε"];
        let after = ["abcd"];
        let actualChanges = getChanges(before, after);

        let expectedChanges =
          ModelContentChange.[
            {range: range(1, 1, 1, 6), text: "abcd", rangeLength: 5},
          ];

        expect.equal(expectedChanges, actualChanges);
      });
    });

    describe("deletion", ({test, _}) => {
      test("delete all lines", ({expect, _}) => {
        let before = ["abc", "def", "ghi"];
        let after = [];
        let actualChanges = getChanges(before, after);

        let expectedChanges =
          ModelContentChange.[
            {range: range(1, 1, 3, 4), text: "", rangeLength: 11},
          ];

        expect.equal(expectedChanges, actualChanges);
      });

      test("delete first line", ({expect, _}) => {
        let before = ["abc", "def", "ghi"];
        let after = ["def", "ghi"];
        let actualChanges = getChanges(before, after);

        let expectedChanges =
          ModelContentChange.[
            {range: range(1, 1, 2, 1), text: "", rangeLength: 4},
          ];

        expect.equal(expectedChanges, actualChanges);
      });
      test("delete last line", ({expect, _}) => {
        let before = ["abc", "def", "ghi"];
        let after = ["abc", "def"];
        let actualChanges = getChanges(before, after);
        let expectedChanges =
          ModelContentChange.[
            {range: range(2, 4, 3, 4), text: "", rangeLength: 4},
          ];
        expect.equal(expectedChanges, actualChanges);
      });
      test("delete last line, with utf-8 characters", ({expect, _}) => {
        let before = ["abc", "κόσμε", "κ"];
        let after = ["abc", "κόσμε"];
        let actualChanges = getChanges(before, after);
        let expectedChanges =
          ModelContentChange.[
            {range: range(2, 6, 3, 2), text: "", rangeLength: 2},
          ];
        expect.equal(expectedChanges, actualChanges);
      });
    });
  })
});
