open Oni_Core;
open TestFramework;

module Buffer = Oni_Core.Buffer;

let makeBuffer = lines => Buffer.ofLines(~font=Font.default(), lines);

describe("DiffMarkers", ({describe, _}) => {
  describe("generate", ({test, _}) => {
    test("single added", ({expect, _}) => {
      let originalLines = [|"a", "b", "c", "d"|];
      let now = [|"a", "b", ".", "c", "d"|];

      let buffer = makeBuffer(now);

      let actual = DiffMarkers.(generate(~originalLines, buffer) |> toArray);
      let expected =
        DiffMarkers.(
          [|Unmodified, Unmodified, Added, Unmodified, Unmodified|]
        );

      expect.array(actual).toEqual(expected);
    });

    test("block added", ({expect, _}) => {
      let originalLines = [|"a", "b", "c", "d"|];
      let now = [|"a", "b", ".", ".", ".", "c", "d"|];

      let buffer = makeBuffer(now);

      let actual = DiffMarkers.(generate(~originalLines, buffer) |> toArray);
      let expected =
        DiffMarkers.(
          [|
            Unmodified,
            Unmodified,
            Added,
            Added,
            Added,
            Unmodified,
            Unmodified,
          |]
        );
      expect.array(actual).toEqual(expected);
    });

    test("single deleted", ({expect, _}) => {
      let originalLines = [|"a", "b", "c", "d"|];
      let now = [|"a", "b", "d"|];

      let buffer = makeBuffer(now);

      let actual = DiffMarkers.(generate(~originalLines, buffer) |> toArray);
      let expected = DiffMarkers.([|Unmodified, Unmodified, DeletedBefore|]);

      expect.array(actual).toEqual(expected);
    });

    test("block deleted from beginning", ({expect, _}) => {
      let originalLines = [|"a", "b", "c", "d"|];
      let now = [|"c", "d"|];

      let buffer = makeBuffer(now);

      let actual = DiffMarkers.(generate(~originalLines, buffer) |> toArray);
      let expected = DiffMarkers.([|DeletedBefore, Unmodified|]);

      expect.array(actual).toEqual(expected);
    });

    test("block deleted from end", ({expect, _}) => {
      let originalLines = [|"a", "b", "c", "d"|];
      let now = [|"a", "b"|];

      let buffer = makeBuffer(now);

      let actual = DiffMarkers.(generate(~originalLines, buffer) |> toArray);
      let expected = DiffMarkers.([|Unmodified, DeletedAfter|]);

      expect.array(actual).toEqual(expected);
    });

    test("block deleted entirely", ({expect, _}) => {
      let originalLines = [|"a", "b", "c", "d"|];
      let now = [||];

      let buffer = makeBuffer(now);

      let actual = DiffMarkers.(generate(~originalLines, buffer) |> toArray);
      let expected = DiffMarkers.([|DeletedBefore|]);

      expect.array(actual).toEqual(expected);
    });

    test("single modified", ({expect, _}) => {
      let originalLines = [|"a", "b", "c", "d"|];
      let now = [|"a", "b", "C", "d"|];

      let buffer = makeBuffer(now);

      let actual = DiffMarkers.(generate(~originalLines, buffer) |> toArray);
      let expected =
        DiffMarkers.([|Unmodified, Unmodified, Modified, Unmodified|]);

      expect.array(actual).toEqual(expected);
    });

    test("block modified", ({expect, _}) => {
      let originalLines = [|"a", "b", "c", "d"|];
      let now = [|"a", "B", "C", "d"|];

      let buffer = makeBuffer(now);

      let actual = DiffMarkers.(generate(~originalLines, buffer) |> toArray);
      let expected =
        DiffMarkers.([|Unmodified, Modified, Modified, Unmodified|]);

      expect.array(actual).toEqual(expected);
    });

    test("add + modify", ({expect, _}) => {
      let originalLines = [|"a", "b", "c", "d"|];
      let now = [|"a", ".", "B", "c", "d"|];

      let buffer = makeBuffer(now);

      let actual = DiffMarkers.(generate(~originalLines, buffer) |> toArray);
      let expected =
        DiffMarkers.([|Unmodified, Modified, Added, Unmodified, Unmodified|]);

      expect.array(actual).toEqual(expected);
    });

    test("delete + modify", ({expect, _}) => {
      let originalLines = [|"a", "b", "c", "d"|];
      let now = [|"a", "C", "d"|];

      let buffer = makeBuffer(now);

      let actual = DiffMarkers.(generate(~originalLines, buffer) |> toArray);
      let expected = DiffMarkers.([|Unmodified, Modified, DeletedBefore|]);

      expect.array(actual).toEqual(expected);
    });

    test("delete + add + delete", ({expect, _}) => {
      let originalLines = [|"a", "b", "c", "d", "e", "f", "g"|];
      let now = [|"a", "c", ".", "d", "f", "g"|];

      let buffer = makeBuffer(now);

      let actual = DiffMarkers.(generate(~originalLines, buffer) |> toArray);
      let expected =
        DiffMarkers.(
          [|
            Unmodified,
            DeletedBefore,
            Added,
            Unmodified,
            DeletedBefore,
            Unmodified,
          |]
        );

      expect.array(actual).toEqual(expected);
    });

    test("delete block + add + delete", ({expect, _}) => {
      let originalLines = [|
        "a",
        "b",
        "c",
        "d",
        "e",
        "f",
        "g",
        "h",
        "i",
        "j",
      |];
      let now = [|"a", "d", ".", "e", "h", "i", "j"|];

      let buffer = makeBuffer(now);

      let actual = DiffMarkers.(generate(~originalLines, buffer) |> toArray);
      let expected =
        DiffMarkers.(
          [|
            Unmodified,
            DeletedBefore,
            Added,
            Unmodified,
            DeletedBefore,
            Unmodified,
            Unmodified,
          |]
        );

      expect.array(actual).toEqual(expected);
    });

    test("delete + add + delete block", ({expect, _}) => {
      let originalLines = [|"a", "b", "c", "d", "e", "f", "g", "h"|];
      let now = [|"a", "c", ".", "d", "g", "h"|];

      let buffer = makeBuffer(now);

      let actual = DiffMarkers.(generate(~originalLines, buffer) |> toArray);
      let expected =
        DiffMarkers.(
          [|
            Unmodified,
            DeletedBefore,
            Added,
            Unmodified,
            DeletedBefore,
            Unmodified,
          |]
        );

      expect.array(actual).toEqual(expected);
    });
  })
});
