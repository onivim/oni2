open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/testfile.txt");
let input = s => ignore(Vim.input(s));
let key = s => ignore(Vim.key(s));

let lineNumberToInt = (lnum: LineNumber.t) => LineNumber.toZeroBased(lnum);
let byteToInt = byte => ByteIndex.toInt(byte);

describe("Visual", ({describe, _}) => {
  describe("getRange", ({test, _}) =>
    test("simple range", ({expect, _}) => {
      let _ = resetBuffer();

      input("v");
      input("l");

      let range = Visual.getRange();
      expect.int(range.start.line |> lineNumberToInt).toBe(0);
      expect.int(range.start.byte |> byteToInt).toBe(0);
      expect.int(range.stop.line |> lineNumberToInt).toBe(0);
      expect.int(range.stop.byte |> byteToInt).toBe(1);

      input("3");
      input("l");

      let range = Visual.getRange();
      expect.int(range.start.line |> lineNumberToInt).toBe(0);
      expect.int(range.start.byte |> byteToInt).toBe(0);
      expect.int(range.stop.line |> lineNumberToInt).toBe(0);
      expect.int(range.stop.byte |> byteToInt).toBe(4);

      input("j");
      let range = Visual.getRange();
      expect.int(range.start.line |> lineNumberToInt).toBe(0);
      expect.int(range.start.byte |> byteToInt).toBe(0);
      expect.int(range.stop.line |> lineNumberToInt).toBe(1);
      expect.int(range.stop.byte |> byteToInt).toBe(4);
    })
  );

  describe("getType", ({test, _}) =>
    test("simple range", ({expect, _}) => {
      let _ = resetBuffer();

      let vt = Visual.getType();
      expect.bool(vt == None).toBe(true);

      input("v");
      let vt = Visual.getType();
      expect.bool(vt == Character).toBe(true);
      key("<esc>");

      input("V");
      let vt = Visual.getType();
      expect.bool(vt == Line).toBe(true);
      key("<esc>");

      key("<C-v>");
      let vt = Visual.getType();
      expect.bool(vt == Block).toBe(true);
      key("<esc>");

      let vt = Visual.getType();
      expect.bool(vt == None).toBe(true);
    })
  );
});
