open EditorCoreTypes;
open TestFramework;

open Vim;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/testfile.txt");

describe("Mode", ({describe, _}) => {
  describe("explicit transitions", ({test, _}) => {
    test("normal -> visual select (line)", ({expect, _}) => {
      let _ = resetBuffer();

      let buffer = Vim.Buffer.getCurrent();
      expect.int(Buffer.getLineCount(buffer)).toBe(3);

      expect.bool(Mode.isNormal(Mode.current())).toBe(true);

      // Force selection mode - and type 'a'
      let (outContext, _: list(Effect.t)) =
        Vim.input(
          ~context={
            ...Vim.Context.current(),
            mode:
              Mode.Select(
                VisualRange.{
                  cursor:
                    BytePosition.{
                      line: LineNumber.zero,
                      byte: ByteIndex.zero,
                    },
                  anchor:
                    BytePosition.{
                      line: LineNumber.ofZeroBased(1),
                      byte: ByteIndex.zero,
                    },
                  visualType: Vim.Types.Line,
                },
              ),
          },
          "a",
        );

      expect.bool(Mode.isInsert(outContext.mode)).toBe(true);

      let buffer = Vim.Buffer.getCurrent();
      expect.int(Buffer.getLineCount(buffer)).toBe(1);
      expect.string(Buffer.getLine(buffer, LineNumber.zero)).toEqual(
        "aThis is the third line of a test file",
      );

      let cursor: BytePosition.t = Cursor.get();
      expect.equal(cursor.byte, ByteIndex.ofInt(1));
      expect.equal(cursor.line, LineNumber.zero);
    });
    test("normal -> visual (character)", ({expect, _}) => {
      let _ = resetBuffer();

      let buffer = Vim.Buffer.getCurrent();
      expect.int(Buffer.getLineCount(buffer)).toBe(3);

      expect.bool(Mode.isNormal(Mode.current())).toBe(true);

      // Force visual mode - and delete with 'x'
      let (outContext, _: list(Effect.t)) =
        Vim.input(
          ~context={
            ...Vim.Context.current(),
            mode:
              Mode.Visual(
                VisualRange.{
                  cursor:
                    BytePosition.{
                      line: LineNumber.zero,
                      byte: ByteIndex.ofInt(4),
                    },
                  anchor:
                    BytePosition.{
                      line: LineNumber.zero,
                      byte: ByteIndex.ofInt(6),
                    },
                  visualType: Vim.Types.Character,
                },
              ),
          },
          "x",
        );

      expect.bool(Mode.isNormal(outContext.mode)).toBe(true);

      let buffer = Vim.Buffer.getCurrent();
      expect.int(Buffer.getLineCount(buffer)).toBe(3);
      expect.string(Buffer.getLine(buffer, LineNumber.ofZeroBased(0))).
        toEqual(
        "This the first line of a test file",
      );

      let cursor: BytePosition.t = Cursor.get();
      expect.equal(cursor.byte, ByteIndex.ofInt(4));
      expect.equal(cursor.line, LineNumber.zero);
    });
  });
  describe("replace mode", ({test, _}) => {
    test("replace mode is reported correctly", ({expect, _}) => {
      let _ = resetBuffer();

      // Enter replace mode
      let _ = Vim.input("R");

      expect.equal(Vim.Mode.isReplace(Vim.Mode.current()), true);
    })
  });
  describe("select mode", ({test, _}) => {
    test("select mode is reported correctly", ({expect, _}) => {
      let _ = resetBuffer();

      // Enter replace mode
      let _ = Vim.input("V");
      expect.equal(Vim.Mode.isVisual(Vim.Mode.current()), true);

      let _ = Vim.key("<C-g>");

      expect.equal(Vim.Mode.isSelect(Vim.Mode.current()), true);
    })
  });

  describe("operator pending mode mode", ({test, _}) => {
    test("d", ({expect, _}) => {
      let _ = resetBuffer();

      // delete pending op
      let _ = Vim.input("d");

      let mode = Vim.Mode.current();
      expect.equal(
        mode,
        Vim.Mode.Operator({
          cursor: BytePosition.zero,
          pending: Vim.Operator.{operation: Delete, count: 0, register: 0},
        }),
      );
    });
    test("cursor position is reported correctly", ({expect, _}) => {
      let _ = resetBuffer();

      // delete pending op
      let ({mode, _}: Vim.Context.t, _: list(Effect.t)) = Vim.input("jjd");

      expect.equal(
        mode,
        Vim.Mode.Operator({
          cursor:
            BytePosition.{
              byte: ByteIndex.zero,
              line: LineNumber.ofZeroBased(2),
            },
          pending: Vim.Operator.{operation: Delete, count: 0, register: 0},
        }),
      );
    });
    test("5d", ({expect, _}) => {
      let _ = resetBuffer();

      // delete pending op, with multiplier
      let _ = Vim.input("5d");

      let mode = Vim.Mode.current();
      expect.equal(
        mode,
        Vim.Mode.Operator({
          cursor: BytePosition.zero,
          pending: Vim.Operator.{operation: Delete, count: 5, register: 0},
        }),
      );
    });
    test("10gc", ({expect, _}) => {
      let _ = resetBuffer();

      // Comment operator
      let _ = Vim.input("10gc");

      let mode = Vim.Mode.current();
      expect.equal(
        mode,
        Vim.Mode.Operator({
          cursor: BytePosition.zero,
          pending: Vim.Operator.{operation: Comment, count: 10, register: 0},
        }),
      );
    });
    test("yank with register", ({expect, _}) => {
      let _ = resetBuffer();

      // Yank to register a
      let _ = Vim.input("\"ay");

      let mode = Vim.Mode.current();
      expect.equal(
        mode,
        Vim.Mode.Operator({
          cursor: BytePosition.zero,
          pending:
            Vim.Operator.{
              operation: Yank,
              count: 0,
              register: int_of_char('a'),
            },
        }),
      );
    });
  });
});
