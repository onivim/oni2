open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/lines_100.txt");
let input = s => ignore(Vim.input(s));

describe("Cursor", ({describe, _}) => {
  describe("setLocation", ({test, _}) => {
    test("cursor location gets updated", ({expect, _}) => {
      let _ = resetBuffer();
      Cursor.set(
        BytePosition.{line: LineNumber.zero, byte: ByteIndex.(zero + 1)},
      );
      expect.int(Cursor.get() |> BytePosition.line |> LineNumber.toZeroBased).
        toBe(
        0,
      );
      expect.int(Cursor.get() |> BytePosition.byte |> ByteIndex.toInt).toBe(
        1,
      );

      Cursor.set(
        BytePosition.{
          line: LineNumber.ofOneBased(3),
          byte: ByteIndex.(zero + 4),
        },
      );
      expect.int(Cursor.get() |> BytePosition.line |> LineNumber.toZeroBased).
        toBe(
        2,
      );
      expect.int(Cursor.get() |> BytePosition.byte |> ByteIndex.toInt).toBe(
        4,
      );
    })
  });
  describe("normal mode", ({test, _}) => {
    test("j / k", ({expect, _}) => {
      let _ = resetBuffer();

      expect.int(Cursor.get() |> BytePosition.line |> LineNumber.toZeroBased).
        toBe(
        0,
      );
      expect.int(Cursor.get() |> BytePosition.byte |> ByteIndex.toInt).toBe(
        0,
      );

      input("j");

      expect.int(Cursor.get() |> BytePosition.line |> LineNumber.toZeroBased).
        toBe(
        1,
      );
      expect.int(Cursor.get() |> BytePosition.byte |> ByteIndex.toInt).toBe(
        0,
      );

      input("j");

      expect.int(Cursor.get() |> BytePosition.line |> LineNumber.toZeroBased).
        toBe(
        2,
      );
      expect.int(Cursor.get() |> BytePosition.byte |> ByteIndex.toInt).toBe(
        0,
      );

      input("2");
      input("k");

      expect.int(Cursor.get() |> BytePosition.line |> LineNumber.toZeroBased).
        toBe(
        0,
      );
      expect.int(Cursor.get() |> BytePosition.byte |> ByteIndex.toInt).toBe(
        0,
      );
    });

    test("gg / G", ({expect, _}) => {
      let _ = resetBuffer();

      expect.int(Cursor.get() |> BytePosition.line |> LineNumber.toZeroBased).
        toBe(
        0,
      );
      expect.int(Cursor.get() |> BytePosition.byte |> ByteIndex.toInt).toBe(
        0,
      );

      input("G");

      expect.int(Cursor.get() |> BytePosition.line |> LineNumber.toZeroBased).
        toBe(
        99,
      );
      expect.int(Cursor.get() |> BytePosition.byte |> ByteIndex.toInt).toBe(
        0,
      );

      input("g");
      input("g");

      expect.int(Cursor.get() |> BytePosition.line |> LineNumber.toZeroBased).
        toBe(
        0,
      );
      expect.int(Cursor.get() |> BytePosition.byte |> ByteIndex.toInt).toBe(
        0,
      );
    });
  });
});
