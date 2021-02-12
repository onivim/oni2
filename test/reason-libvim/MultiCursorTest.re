open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/testfile.txt");

let input =
    (
      ~autoClosingPairs=AutoClosingPairs.empty,
      ~mode=Mode.Normal({cursor: BytePosition.zero}),
      key,
    ) => {
  let (out, _effects: list(Effect.t)) =
    Vim.input(~context={...Context.current(), autoClosingPairs, mode}, key);

  Context.(out.mode);
};

let key =
    (
      ~autoClosingPairs=AutoClosingPairs.empty,
      ~mode=Mode.Insert({cursors: []}),
      key,
    ) => {
  let (out, _effects: list(Effect.t)) =
    Vim.key(
      ~context=Context.{...Context.current(), autoClosingPairs, mode},
      key,
    );

  Context.(out.mode);
};
let hasCursorMatching = (~lineIndex, ~byteIndex, cursors) => {
  EditorCoreTypes.(
    cursors
    |> List.exists((cursor: BytePosition.t) =>
         LineNumber.toZeroBased(cursor.line) == lineIndex
         && ByteIndex.toInt(cursor.byte) == byteIndex
       )
  );
};

describe("Multi-cursor", ({describe, _}) => {
  describe("multi-selection", ({test, _}) => {
    // A set of ranges to add cursors for the test file:
    // [This] [is] [the] first line of a test file
    let sameLineRanges =
      VisualRange.[
        {
          visualType: Vim.Types.Character,
          cursor: BytePosition.{line: LineNumber.zero, byte: ByteIndex.zero},
          anchor:
            BytePosition.{line: LineNumber.zero, byte: ByteIndex.ofInt(3)},
        },
        {
          visualType: Vim.Types.Character,
          cursor:
            BytePosition.{line: LineNumber.(zero), byte: ByteIndex.ofInt(5)},
          anchor:
            BytePosition.{line: LineNumber.(zero), byte: ByteIndex.ofInt(6)},
        },
        {
          visualType: Vim.Types.Character,
          cursor:
            BytePosition.{line: LineNumber.(zero), byte: ByteIndex.ofInt(8)},
          anchor:
            BytePosition.{
              line: LineNumber.(zero),
              byte: ByteIndex.ofInt(10),
            },
        },
      ];
    test("multiple selection -> multiple insert mode cursors", ({expect, _}) => {
      let buffer = resetBuffer();

      expect.int(Buffer.getLineCount(buffer)).toBe(3);

      expect.bool(Mode.isNormal(Mode.current())).toBe(true);

      let ranges: list(VisualRange.t) =
        VisualRange.[
          {
            visualType: Vim.Types.Character,
            cursor:
              BytePosition.{line: LineNumber.zero, byte: ByteIndex.zero},
            anchor:
              BytePosition.{line: LineNumber.zero, byte: ByteIndex.ofInt(3)},
          },
          {
            visualType: Vim.Types.Character,
            cursor:
              BytePosition.{
                line: LineNumber.(zero + 1),
                byte: ByteIndex.zero,
              },
            anchor:
              BytePosition.{
                line: LineNumber.(zero + 1),
                byte: ByteIndex.ofInt(3),
              },
          },
          {
            visualType: Vim.Types.Character,
            cursor:
              BytePosition.{
                line: LineNumber.(zero + 2),
                byte: ByteIndex.zero,
              },
            anchor:
              BytePosition.{
                line: LineNumber.(zero + 2),
                byte: ByteIndex.ofInt(3),
              },
          },
        ];

      // Force selection mode with 3 ranges
      let (outContext, _: list(Effect.t)) =
        Vim.input(
          ~context={
            ...Vim.Context.current(),
            mode: Mode.Select({ranges: ranges}),
          },
          "a",
        );

      // Verify we've transitioned to insert mode
      expect.bool(Mode.isInsert(outContext.mode)).toBe(true);

      let cursors = Mode.cursors(outContext.mode);

      // Verify we now have 3 cursors
      expect.equal(
        cursors |> hasCursorMatching(~lineIndex=0, ~byteIndex=1),
        true,
      );

      expect.equal(
        cursors |> hasCursorMatching(~lineIndex=1, ~byteIndex=1),
        true,
      );
      expect.equal(
        cursors |> hasCursorMatching(~lineIndex=2, ~byteIndex=1),
        true,
      );

      // Verify buffer contents
      expect.string(Buffer.getLine(buffer, LineNumber.zero)).toEqual(
        "a is the first line of a test file",
      );
      expect.string(Buffer.getLine(buffer, LineNumber.(zero + 1))).toEqual(
        "a is the second line of a test file",
      );
      expect.string(Buffer.getLine(buffer, LineNumber.(zero + 2))).toEqual(
        "a is the third line of a test file",
      );

      // Entering another character should update all lines
      let (_: Vim.Context.t, _: list(Effect.t)) =
        Vim.input(~context=outContext, "b");
      expect.string(Buffer.getLine(buffer, LineNumber.zero)).toEqual(
        "ab is the first line of a test file",
      );
      expect.string(Buffer.getLine(buffer, LineNumber.(zero + 1))).toEqual(
        "ab is the second line of a test file",
      );
      expect.string(Buffer.getLine(buffer, LineNumber.(zero + 2))).toEqual(
        "ab is the third line of a test file",
      );
    });
    test(
      "multiple selection -> multiple insert mode cursors - same line (forward order)",
      ({expect, _}) => {
        let buffer = resetBuffer();

        expect.int(Buffer.getLineCount(buffer)).toBe(3);

        expect.bool(Mode.isNormal(Mode.current())).toBe(true);

        let ranges = sameLineRanges;

        // Force selection mode with 3 ranges
        let (outContext, _: list(Effect.t)) =
          Vim.input(
            ~context={
              ...Vim.Context.current(),
              mode: Mode.Select({ranges: ranges}),
            },
            "a",
          );

        // Verify we've transitioned to insert mode
        expect.bool(Mode.isInsert(outContext.mode)).toBe(true);

        let cursors = Mode.cursors(outContext.mode);

        // Verify we now have 3 cursors
        expect.equal(
          cursors |> hasCursorMatching(~lineIndex=0, ~byteIndex=1),
          true,
        );

        expect.equal(
          cursors |> hasCursorMatching(~lineIndex=0, ~byteIndex=3),
          true,
        );
        expect.equal(
          cursors |> hasCursorMatching(~lineIndex=0, ~byteIndex=5),
          true,
        );
        // Verify buffer contents
        expect.string(Buffer.getLine(buffer, LineNumber.zero)).toEqual(
          "a a a first line of a test file",
        );
      },
    );
    test(
      "multiple selection -> multiple insert mode cursors - same line (reverse order)",
      ({expect, _}) => {
        let buffer = resetBuffer();

        expect.int(Buffer.getLineCount(buffer)).toBe(3);

        expect.bool(Mode.isNormal(Mode.current())).toBe(true);

        let ranges = sameLineRanges |> List.rev;

        // Force selection mode with 3 ranges
        let (outContext, _: list(Effect.t)) =
          Vim.input(
            ~context={
              ...Vim.Context.current(),
              mode: Mode.Select({ranges: ranges}),
            },
            "a",
          );

        // Verify we've transitioned to insert mode
        expect.bool(Mode.isInsert(outContext.mode)).toBe(true);

        let cursors = Mode.cursors(outContext.mode);
        expect.int(List.length(cursors)).toBe(3);

        // Verify we now have 3 cursors
        expect.equal(
          cursors |> hasCursorMatching(~lineIndex=0, ~byteIndex=1),
          true,
        );

        expect.equal(
          cursors |> hasCursorMatching(~lineIndex=0, ~byteIndex=3),
          true,
        );
        expect.equal(
          cursors |> hasCursorMatching(~lineIndex=0, ~byteIndex=5),
          true,
        );
        // Verify buffer contents
        expect.string(Buffer.getLine(buffer, LineNumber.zero)).toEqual(
          "a a a first line of a test file",
        );
      },
    );
    test(
      "multiple selection -> multiple insert mode cursors - same line (multi-byte character)",
      ({expect, _}) => {
        let buffer = resetBuffer();

        expect.int(Buffer.getLineCount(buffer)).toBe(3);

        expect.bool(Mode.isNormal(Mode.current())).toBe(true);

        let ranges = sameLineRanges;

        // Force selection mode with 3 ranges
        let (outContext, _: list(Effect.t)) =
          Vim.input(
            ~context={
              ...Vim.Context.current(),
              mode: Mode.Select({ranges: ranges}),
            },
            "ό" // 3-byte character
          );

        // Verify we've transitioned to insert mode
        expect.bool(Mode.isInsert(outContext.mode)).toBe(true);

        let cursors = Mode.cursors(outContext.mode);

        // Verify we now have 3 cursors
        expect.equal(
          cursors |> hasCursorMatching(~lineIndex=0, ~byteIndex=3),
          true,
        );

        expect.equal(
          cursors |> hasCursorMatching(~lineIndex=0, ~byteIndex=7),
          true,
        );
        expect.equal(
          cursors |> hasCursorMatching(~lineIndex=0, ~byteIndex=11),
          true,
        );
        // Verify buffer contents
        expect.string(Buffer.getLine(buffer, LineNumber.zero)).toEqual(
          "ό ό ό first line of a test file",
        );
      },
    );
  });
  describe("visual block mode", ({test, _}) => {
    test("expand to multiple cursors with 'I'", ({expect, _}) => {
      let _: Buffer.t = resetBuffer();
      let (context, _) = Vim.key("<c-v>");
      let (context, _) = Vim.input(~context, "j");
      let (context, _) = Vim.input(~context, "j");

      expect.equal(Vim.Mode.isVisual(context.mode), true);

      let (context, _) = Vim.input(~context, "I");

      expect.equal(Vim.Mode.isInsert(context.mode), true);

      let cursors = Vim.Mode.cursors(context.mode);
      expect.equal(List.length(cursors), 3);

      expect.equal(
        cursors |> hasCursorMatching(~lineIndex=0, ~byteIndex=0),
        true,
      );
      expect.equal(
        cursors |> hasCursorMatching(~lineIndex=1, ~byteIndex=0),
        true,
      );
      expect.equal(
        cursors |> hasCursorMatching(~lineIndex=2, ~byteIndex=0),
        true,
      );
    });
    test("expand to multiple cursors with 'A'", ({expect, _}) => {
      let _: Buffer.t = resetBuffer();
      let (context, _) = Vim.key("<c-v>");
      let (context, _) = Vim.input(~context, "j");
      let (context, _) = Vim.input(~context, "j");

      expect.equal(Vim.Mode.isVisual(context.mode), true);

      let (context, _) = Vim.input(~context, "A");

      expect.equal(Vim.Mode.isInsert(context.mode), true);

      let cursors = Vim.Mode.cursors(context.mode);
      expect.equal(List.length(cursors), 3);

      expect.equal(
        cursors |> hasCursorMatching(~lineIndex=0, ~byteIndex=1),
        true,
      );
      expect.equal(
        cursors |> hasCursorMatching(~lineIndex=1, ~byteIndex=1),
        true,
      );
      expect.equal(
        cursors |> hasCursorMatching(~lineIndex=2, ~byteIndex=1),
        true,
      );
    });
  });
  describe("normal mode", ({describe, _}) => {
    describe("single cursor", ({test, _}) => {
      test("set cursor works as expected", ({expect, _}) => {
        let _: Buffer.t = resetBuffer();
        let mode1 = input("j");

        mode1
        |> Vim.Mode.cursors
        |> List.hd
        |> (
          cursor => expect.int(cursor.line |> LineNumber.toZeroBased).toBe(1)
        );

        expect.int(
          Cursor.get() |> BytePosition.line |> LineNumber.toZeroBased,
        ).
          toBe(
          1,
        );

        // set cursor, and move up
        let mode2 =
          input(
            ~mode=
              Normal({
                cursor:
                  BytePosition.{
                    line: LineNumber.ofZeroBased(2),
                    byte: ByteIndex.zero,
                  },
              }),
            "k",
          );

        mode2
        |> Vim.Mode.cursors
        |> List.hd
        |> (
          cursor =>
            expect.int(cursor |> BytePosition.line |> LineNumber.toZeroBased).
              toBe(
              1,
            )
        );

        expect.int(
          Cursor.get() |> BytePosition.line |> LineNumber.toZeroBased,
        ).
          toBe(
          1,
        );
      })
    })
  });
  describe("insert mode", ({describe, test, _}) => {
    describe("undo", ({test, _}) => {
      test("undo multiple lines", ({expect, _}) => {
        let buf = resetBuffer();
        let mode =
          input(
            ~mode=
              Vim.Mode.Insert({
                cursors: [
                  BytePosition.{line: LineNumber.zero, byte: ByteIndex.zero},
                  BytePosition.{
                    line: LineNumber.(zero + 1),
                    byte: ByteIndex.zero,
                  },
                  BytePosition.{
                    line: LineNumber.(zero + 2),
                    byte: ByteIndex.zero,
                  },
                ],
              }),
            "a",
          );

        let line1 = Buffer.getLine(buf, LineNumber.zero);
        let line2 = Buffer.getLine(buf, LineNumber.(zero + 1));
        let line3 = Buffer.getLine(buf, LineNumber.(zero + 2));

        expect.string(line1).toEqual(
          "aThis is the first line of a test file",
        );
        expect.string(line2).toEqual(
          "aThis is the second line of a test file",
        );
        expect.string(line3).toEqual(
          "aThis is the third line of a test file",
        );

        let mode' = key(~mode, "<esc>");
        let _mode'' = key(~mode=mode', "u");

        let line1 = Buffer.getLine(buf, LineNumber.zero);
        let line2 = Buffer.getLine(buf, LineNumber.(zero + 1));
        let line3 = Buffer.getLine(buf, LineNumber.(zero + 2));

        expect.string(line1).toEqual(
          "This is the first line of a test file",
        );
        expect.string(line2).toEqual(
          "This is the second line of a test file",
        );
        expect.string(line3).toEqual(
          "This is the third line of a test file",
        );
      })
    });
    test("multi-cursor auto-closing pairs", ({expect, _}) => {
      let buf = resetBuffer();

      let autoClosingPairs =
        AutoClosingPairs.create(
          AutoClosingPairs.[AutoPair.{opening: "{", closing: "}"}],
        );

      let updates: ref(list(BufferUpdate.t)) = ref([]);
      let dispose = Buffer.onUpdate(upd => updates := [upd, ...updates^]);

      let (_context: Context.t, _effects: list(Effect.t)) = Vim.input("i");
      expect.int(List.length(updates^)).toBe(0);

      let mode =
        input(
          ~autoClosingPairs,
          ~mode=
            Vim.Mode.Insert({
              cursors: [
                BytePosition.{line: LineNumber.zero, byte: ByteIndex.zero},
                BytePosition.{
                  line: LineNumber.(zero + 1),
                  byte: ByteIndex.zero,
                },
                BytePosition.{
                  line: LineNumber.(zero + 2),
                  byte: ByteIndex.zero,
                },
              ],
            }),
          "{",
        );

      let line1 = Buffer.getLine(buf, LineNumber.zero);
      let line2 = Buffer.getLine(buf, LineNumber.(zero + 1));
      let line3 = Buffer.getLine(buf, LineNumber.(zero + 2));

      expect.string(line1).toEqual(
        "{}This is the first line of a test file",
      );
      expect.string(line2).toEqual(
        "{}This is the second line of a test file",
      );
      expect.string(line3).toEqual(
        "{}This is the third line of a test file",
      );

      let _: Vim.Mode.t = input(~autoClosingPairs, ~mode, "a");

      let line1 = Buffer.getLine(buf, LineNumber.zero);
      let line2 = Buffer.getLine(buf, LineNumber.(zero + 1));
      let line3 = Buffer.getLine(buf, LineNumber.(zero + 2));

      expect.string(line1).toEqual(
        "{a}This is the first line of a test file",
      );
      expect.string(line2).toEqual(
        "{a}This is the second line of a test file",
      );
      expect.string(line3).toEqual(
        "{a}This is the third line of a test file",
      );

      dispose();
    });
    test("multi-cursor auto-closing pairs, same line", ({expect, _}) => {
      let buf = resetBuffer();

      let autoClosingPairs =
        AutoClosingPairs.create(
          ~allowBefore=[" "],
          AutoClosingPairs.[AutoPair.{opening: "{", closing: "}"}],
        );
      let mode =
        input(
          ~autoClosingPairs,
          ~mode=
            Vim.Mode.Insert({
              cursors: [
                BytePosition.{line: LineNumber.zero, byte: ByteIndex.zero},
                BytePosition.{
                  line: LineNumber.(zero),
                  byte: ByteIndex.(zero + 4),
                },
                BytePosition.{
                  line: LineNumber.(zero),
                  byte: ByteIndex.(zero + 7),
                },
              ],
            }),
          "{",
        );

      let line1 = Buffer.getLine(buf, LineNumber.zero);

      expect.string(line1).toEqual(
        "{}This{} is{} the first line of a test file",
      );

      let _: Vim.Mode.t = input(~autoClosingPairs, ~mode, "a");

      let line1 = Buffer.getLine(buf, LineNumber.zero);

      expect.string(line1).toEqual(
        "{a}This{a} is{a} the first line of a test file",
      );
    });
    test("insert / backspace", ({expect, _}) => {
      let buf = resetBuffer();

      let updates: ref(list(BufferUpdate.t)) = ref([]);
      let dispose = Buffer.onUpdate(upd => updates := [upd, ...updates^]);

      let (_: Context.t, _: list(Effect.t)) = Vim.input("i");
      expect.int(List.length(updates^)).toBe(0);

      let mode =
        input(
          ~mode=
            Insert({
              cursors: [
                BytePosition.{line: LineNumber.zero, byte: ByteIndex.zero},
                BytePosition.{
                  line: LineNumber.(zero + 1),
                  byte: ByteIndex.zero,
                },
                BytePosition.{
                  line: LineNumber.(zero + 2),
                  byte: ByteIndex.zero,
                },
              ],
            }),
          "a",
        );

      let line1 = Buffer.getLine(buf, LineNumber.zero);
      let line2 = Buffer.getLine(buf, LineNumber.(zero + 1));
      let line3 = Buffer.getLine(buf, LineNumber.(zero + 2));

      expect.string(line1).toEqual("aThis is the first line of a test file");
      expect.string(line2).toEqual(
        "aThis is the second line of a test file",
      );
      expect.string(line3).toEqual("aThis is the third line of a test file");

      let mode = input(~mode, "b");

      let line1 = Buffer.getLine(buf, LineNumber.zero);
      let line2 = Buffer.getLine(buf, LineNumber.(zero + 1));
      let line3 = Buffer.getLine(buf, LineNumber.(zero + 2));

      expect.string(line1).toEqual(
        "abThis is the first line of a test file",
      );
      expect.string(line2).toEqual(
        "abThis is the second line of a test file",
      );
      expect.string(line3).toEqual(
        "abThis is the third line of a test file",
      );

      let _: Vim.Mode.t = key(~mode, "<bs>");

      let line1 = Buffer.getLine(buf, LineNumber.zero);
      let line2 = Buffer.getLine(buf, LineNumber.(zero + 1));
      let line3 = Buffer.getLine(buf, LineNumber.(zero + 2));

      expect.string(line1).toEqual("aThis is the first line of a test file");
      expect.string(line2).toEqual(
        "aThis is the second line of a test file",
      );
      expect.string(line3).toEqual("aThis is the third line of a test file");

      dispose();
    });
    test("multiple cursor, same line", ({expect, _}) => {
      let buf = resetBuffer();

      let (_: Context.t, _: list(Effect.t)) = Vim.input("i");

      let mode =
        input(
          ~mode=
            Insert({
              cursors: [
                BytePosition.{line: LineNumber.zero, byte: ByteIndex.zero},
                BytePosition.{
                  line: LineNumber.(zero),
                  byte: ByteIndex.(zero + 5),
                },
                BytePosition.{
                  line: LineNumber.(zero),
                  byte: ByteIndex.(zero + 8),
                },
              ],
            }),
          "a",
        );

      let line = Buffer.getLine(buf, LineNumber.zero);

      expect.string(line).toEqual(
        "aThis ais athe first line of a test file",
      );

      let mode =
        input(
          ~mode,
          "ό" // 3-byte character
        );

      let line = Buffer.getLine(buf, LineNumber.zero);
      expect.string(line).toEqual(
        "aόThis aόis aόthe first line of a test file",
      );

      let _: Vim.Mode.t = key(~mode, "<bs>");

      let line = Buffer.getLine(buf, LineNumber.zero);

      expect.string(line).toEqual(
        "aThis ais athe first line of a test file",
      );
    });
  });
});
