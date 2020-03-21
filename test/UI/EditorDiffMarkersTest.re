/* open Oni_Core; */
open TestFramework;

module Buffer = Oni_Core.Buffer;
module EditorDiffMarkers = Feature_Editor.EditorDiffMarkers;

describe("EditorDiffMarkers", ({describe, _}) => {
  describe("generate", ({test, _}) => {
    test("single added", ({expect, _}) => {
      let was = [|"a", "b", "c", "d"|];
      let now = [|"a", "b", ".", "c", "d"|];

      let buffer = Buffer.ofLines(now) |> Buffer.setOriginalLines(was);

      let actual = EditorDiffMarkers.generate(buffer);
      let expected =
        EditorDiffMarkers.(
          [|Unmodified, Unmodified, Added, Unmodified, Unmodified|]
        );

      switch (actual) {
      | Some(actual) => expect.array(actual).toEqual(expected)
      | None => failwith("unreachable")
      };
    });

    test("block added", ({expect, _}) => {
      let was = [|"a", "b", "c", "d"|];
      let now = [|"a", "b", ".", ".", ".", "c", "d"|];

      let buffer = Buffer.ofLines(now) |> Buffer.setOriginalLines(was);

      let actual = EditorDiffMarkers.generate(buffer);
      let expected =
        EditorDiffMarkers.(
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

      switch (actual) {
      | Some(actual) => expect.array(actual).toEqual(expected)
      | None => failwith("unreachable")
      };
    });

    test("single deleted", ({expect, _}) => {
      let was = [|"a", "b", "c", "d"|];
      let now = [|"a", "b", "d"|];

      let buffer = Buffer.ofLines(now) |> Buffer.setOriginalLines(was);

      let actual = EditorDiffMarkers.generate(buffer);
      let expected =
        EditorDiffMarkers.([|Unmodified, Unmodified, DeletedBefore|]);

      switch (actual) {
      | Some(actual) => expect.array(actual).toEqual(expected)
      | None => failwith("unreachable")
      };
    });

    test("block deleted from beginning", ({expect, _}) => {
      let was = [|"a", "b", "c", "d"|];
      let now = [|"c", "d"|];

      let buffer = Buffer.ofLines(now) |> Buffer.setOriginalLines(was);

      let actual = EditorDiffMarkers.generate(buffer);
      let expected = EditorDiffMarkers.([|DeletedBefore, Unmodified|]);

      switch (actual) {
      | Some(actual) => expect.array(actual).toEqual(expected)
      | None => failwith("unreachable")
      };
    });

    test("block deleted from end", ({expect, _}) => {
      let was = [|"a", "b", "c", "d"|];
      let now = [|"a", "b"|];

      let buffer = Buffer.ofLines(now) |> Buffer.setOriginalLines(was);

      let actual = EditorDiffMarkers.generate(buffer);
      let expected = EditorDiffMarkers.([|Unmodified, DeletedAfter|]);

      switch (actual) {
      | Some(actual) => expect.array(actual).toEqual(expected)
      | None => failwith("unreachable")
      };
    });

    test("block deleted entirely", ({expect, _}) => {
      let was = [|"a", "b", "c", "d"|];
      let now = [||];

      let buffer = Buffer.ofLines(now) |> Buffer.setOriginalLines(was);

      let actual = EditorDiffMarkers.generate(buffer);
      let expected = EditorDiffMarkers.([|DeletedBefore|]);

      switch (actual) {
      | Some(actual) => expect.array(actual).toEqual(expected)
      | None => failwith("unreachable")
      };
    });

    test("single modified", ({expect, _}) => {
      let was = [|"a", "b", "c", "d"|];
      let now = [|"a", "b", "C", "d"|];

      let buffer = Buffer.ofLines(now) |> Buffer.setOriginalLines(was);

      let actual = EditorDiffMarkers.generate(buffer);
      let expected =
        EditorDiffMarkers.([|Unmodified, Unmodified, Modified, Unmodified|]);

      switch (actual) {
      | Some(actual) => expect.array(actual).toEqual(expected)
      | None => failwith("unreachable")
      };
    });

    test("block modified", ({expect, _}) => {
      let was = [|"a", "b", "c", "d"|];
      let now = [|"a", "B", "C", "d"|];

      let buffer = Buffer.ofLines(now) |> Buffer.setOriginalLines(was);

      let actual = EditorDiffMarkers.generate(buffer);
      let expected =
        EditorDiffMarkers.([|Unmodified, Modified, Modified, Unmodified|]);

      switch (actual) {
      | Some(actual) => expect.array(actual).toEqual(expected)
      | None => failwith("unreachable")
      };
    });

    test("add + modify", ({expect, _}) => {
      let was = [|"a", "b", "c", "d"|];
      let now = [|"a", ".", "B", "c", "d"|];

      let buffer = Buffer.ofLines(now) |> Buffer.setOriginalLines(was);

      let actual = EditorDiffMarkers.generate(buffer);
      let expected =
        EditorDiffMarkers.(
          [|Unmodified, Modified, Added, Unmodified, Unmodified|]
        );

      switch (actual) {
      | Some(actual) => expect.array(actual).toEqual(expected)
      | None => failwith("unreachable")
      };
    });

    test("delete + modify", ({expect, _}) => {
      let was = [|"a", "b", "c", "d"|];
      let now = [|"a", "C", "d"|];

      let buffer = Buffer.ofLines(now) |> Buffer.setOriginalLines(was);

      let actual = EditorDiffMarkers.generate(buffer);
      let expected =
        EditorDiffMarkers.([|Unmodified, Modified, DeletedBefore|]);

      switch (actual) {
      | Some(actual) => expect.array(actual).toEqual(expected)
      | None => failwith("unreachable")
      };
    });

    test("delete + add + delete", ({expect, _}) => {
      let was = [|"a", "b", "c", "d", "e", "f", "g"|];
      let now = [|"a", "c", ".", "d", "f", "g"|];

      let buffer = Buffer.ofLines(now) |> Buffer.setOriginalLines(was);

      let actual = EditorDiffMarkers.generate(buffer);
      let expected =
        EditorDiffMarkers.(
          [|
            Unmodified,
            DeletedBefore,
            Added,
            Unmodified,
            DeletedBefore,
            Unmodified,
          |]
        );

      switch (actual) {
      | Some(actual) => expect.array(actual).toEqual(expected)
      | None => failwith("unreachable")
      };
    });

    test("delete block + add + delete", ({expect, _}) => {
      let was = [|"a", "b", "c", "d", "e", "f", "g", "h", "i", "j"|];
      let now = [|"a", "d", ".", "e", "h", "i", "j"|];

      let buffer = Buffer.ofLines(now) |> Buffer.setOriginalLines(was);

      let actual = EditorDiffMarkers.generate(buffer);
      let expected =
        EditorDiffMarkers.(
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

      switch (actual) {
      | Some(actual) =>
        // Printf.printf("\n%s\n%!", EditorDiffMarkers.show(actual));
        expect.array(actual).toEqual(expected)
      | None => failwith("unreachable")
      };
    });

    test("delete + add + delete block", ({expect, _}) => {
      let was = [|"a", "b", "c", "d", "e", "f", "g", "h"|];
      let now = [|"a", "c", ".", "d", "g", "h"|];

      let buffer = Buffer.ofLines(now) |> Buffer.setOriginalLines(was);

      let actual = EditorDiffMarkers.generate(buffer);
      let expected =
        EditorDiffMarkers.(
          [|
            Unmodified,
            DeletedBefore,
            Added,
            Unmodified,
            DeletedBefore,
            Unmodified,
          |]
        );

      switch (actual) {
      | Some(actual) => expect.array(actual).toEqual(expected)
      | None => failwith("unreachable")
      };
    });
  })
});
