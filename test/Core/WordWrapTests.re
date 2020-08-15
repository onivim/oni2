open Oni_Core;
open TestFramework;

let makeLine = BufferLine.make(~indentation=IndentationSettings.default);

describe("WordWrap", ({describe, _}) =>
  describe("fixed", ({test, _}) => {
    test("ascii line within wrap point", ({expect, _}) => {
      let line = "abc" |> makeLine;
      let wrap = WordWrap.fixed(~columns=3, line);
      expect.equal(wrap, [{byte: 0, index: 0}]);
    });
    test("ascii line exceeds wrap point", ({expect, _}) => {
      let line = "abcdef" |> makeLine;
      let wrap = WordWrap.fixed(~columns=3, line);
      expect.equal(wrap, [{byte: 0, index: 0}, {byte: 3, index: 3}]);
    });
  })
);
