open Oni_Core;
open Feature_Editor;
open TestFramework;

open Editor;

let simpleAsciiBuffer =
  [|"abcdef", "ghijkl"|] |> Oni_Core.Buffer.ofLines |> EditorBuffer.ofBuffer;

let editor = (~wrap, lines) => {
  let buffer =
    lines |> Array.of_list |> Oni_Core.Buffer.ofLines |> EditorBuffer.ofBuffer;

  let font = Service_Font.default;

  Editor.create(~wrap, ~font, ~buffer, ());
};

describe("Editor", ({describe, _}) => {
  describe("bufferLineByteToPixel", ({test, _}) => {
    test("first line, byte should be at position (nowrap)", ({expect, _}) => {
      let editor = editor(~wrap=WordWrap.none, ["abcdef"]);
      let ({pixelX, pixelY}, _: float) =
        Editor.bufferLineByteToPixel(~line=0, ~byteIndex=0, editor);

      expect.float(pixelX).toBeCloseTo(0.5);
      expect.float(pixelY).toBeCloseTo(0.5);
    });
    test(
      "first line, last byte should be at end position (nowrap)",
      ({expect, _}) => {
      let editor = editor(~wrap=WordWrap.none, ["abcdef"]);
      let characterWidth = Editor.getCharacterWidth(editor);
      let ({pixelX, pixelY}, _: float) =
        Editor.bufferLineByteToPixel(~line=0, ~byteIndex=5, editor);

      expect.float(pixelX).toBeCloseTo(characterWidth *. 5.0 +. 0.5);
      expect.float(pixelY).toBeCloseTo(0.5);
    });
    test("should wrap to next line", ({expect, _}) => {
      let editor = editor(~wrap=WordWrap.fixed(~columns=3), ["abcdef"]);
      let characterWidth = Editor.getCharacterWidth(editor);
      let lineHeight = Editor.getLineHeight(editor);
      let ({pixelX, pixelY}, _: float) =
        Editor.bufferLineByteToPixel(~line=0, ~byteIndex=5, editor);

      expect.float(pixelX).toBeCloseTo(characterWidth *. 2.0 +. 0.5);
      expect.float(pixelY).toBeCloseTo(lineHeight *. 1.0 +. 0.5);
    });
  })
});
