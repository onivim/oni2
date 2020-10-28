open Oni_Core;
open EditorCoreTypes;
open TestFramework;

module Buffer = Oni_Core.Buffer;
module BufferLineColorizer = Feature_Editor.BufferLineColorizer;
module BufferViewTokenizer = Feature_Editor.BufferViewTokenizer;
module Editor = Feature_Editor.Editor;
module EditorBuffer = Feature_Editor.EditorBuffer;
module WrapMode = Editor.WrapMode;

open Oni_Core_Test.Helpers;

describe("Editor", ({describe, _}) => {
  let config = (~vimSetting as _, _key) => Config.NotSet;

  let create = lines => {
    let buffer = lines |> Buffer.ofLines(~font=Font.default());

    let editorBuffer = buffer |> EditorBuffer.ofBuffer;

    (Editor.create(~config, ~buffer=editorBuffer, ()), buffer);
  };

  let (_editor, measureBuffer) = create([||]);
  let aWidth = Buffer.measure(Uchar.of_char('a'), measureBuffer);

  let bytePos = (lnum, byteNum) =>
    BytePosition.{
      line: LineNumber.ofZeroBased(lnum),
      byte: ByteIndex.ofInt(byteNum),
    };
  describe("viewTokens", ({test, _}) => {
    test("single token returned", ({expect, _}) => {
      let (editor, _buffer) = create([|"aaa"|]);
      let editor =
        editor
        |> Editor.setWrapMode(~wrapMode=WrapMode.NoWrap)
        |> Editor.setSize(~pixelWidth=500, ~pixelHeight=500);

      let colorizer = (~startByte as _, _) => {
        BufferLineColorizer.{
          color: Revery.Colors.black,
          backgroundColor: Revery.Colors.white,
          italic: false,
          bold: false,
        };
      };
      let viewTokens =
        Editor.viewTokens(~line=0, ~scrollX=0., ~colorizer, editor);
      expect.int(List.length(viewTokens)).toBe(1);

      let item: BufferViewTokenizer.t = List.nth(viewTokens, 0);
      expect.float(item.startPixel).toBeCloseTo(0.0);
    });
    test("single token returned, with a little bit of scroll", ({expect, _}) => {
      let (editor, _buffer) = create([|"aaa"|]);
      let editor =
        editor
        |> Editor.setWrapMode(~wrapMode=WrapMode.NoWrap)
        |> Editor.setSize(~pixelWidth=500, ~pixelHeight=500);

      let colorizer = (~startByte as _, _) => {
        BufferLineColorizer.{
          color: Revery.Colors.black,
          backgroundColor: Revery.Colors.white,
          italic: false,
          bold: false,
        };
      };
      let viewTokens =
        Editor.viewTokens(~line=0, ~scrollX=1., ~colorizer, editor);
      expect.int(List.length(viewTokens)).toBe(1);

      let item: BufferViewTokenizer.t = List.nth(viewTokens, 0);
      expect.float(item.startPixel).toBeCloseTo(-1.0);
    });
  });

  describe("bufferLineByteToPixel", ({test, _}) => {
    test("first line, byte should be at position (nowrap)", ({expect, _}) => {
      let (editor, _buffer) = create([|"aaaaaa"|]);
      let editor = editor |> Editor.setWrapMode(~wrapMode=WrapMode.NoWrap);
      let ({x, y}: PixelPosition.t, _) =
        Editor.bufferBytePositionToPixel(~position=bytePos(0, 0), editor);
      expect.float(x).toBeCloseTo(0.5);
      expect.float(y).toBeCloseTo(0.5);

      let ({x, y}: PixelPosition.t, _) =
        Editor.bufferBytePositionToPixel(~position=bytePos(0, 5), editor);
      expect.float(x).toBeCloseTo(aWidth *. 5. +. 0.5);
      expect.float(y).toBeCloseTo(0.5);
    });

    test("line wraps halfway (fixed: pixels=3a)", ({expect, _}) => {
      let (editor, _buffer) = create([|"aaaaaa"|]);
      let editor =
        editor
        |> Editor.setMinimap(~enabled=false, ~maxColumn=0)
        |> Editor.setLineNumbers(~lineNumbers=`Off)
        |> Editor.setSize(
             ~pixelWidth=
               int_of_float(
                 3. *. aWidth +. 1.0 +. float(Constants.scrollBarThickness),
               ),
             ~pixelHeight=500,
           )
        |> Editor.setWrapMode(~wrapMode=WrapMode.Viewport);
      let lineHeight = Editor.lineHeightInPixels(editor);
      let ({x, y}: PixelPosition.t, _) =
        Editor.bufferBytePositionToPixel(~position=bytePos(0, 0), editor);
      expect.float(x).toBeCloseTo(0.5);
      expect.float(y).toBeCloseTo(0.5);

      let ({x, y}: PixelPosition.t, _) =
        Editor.bufferBytePositionToPixel(~position=bytePos(0, 3), editor);
      expect.float(x).toBeCloseTo(0.5);
      expect.float(y).toBeCloseTo(lineHeight +. 0.5);
    });
  });

  describe("pixelPositionToBytePosition", ({test, _}) => {
    test("~allowPast boundary cases", ({expect, _}) => {
      let (editor, _buffer) = create([|"abc"|]);

      let lineHeight = editor |> Editor.lineHeightInPixels;

      let (pos, width) =
        Editor.bufferBytePositionToPixel(
          ~position=
            BytePosition.{
              line: LineNumber.ofZeroBased(0),
              byte: ByteIndex.ofInt(2),
            },
          editor,
        );

      let pixelY = pos.y +. lineHeight /. 2.;
      // We want our pixel x to exceed the last character
      let pixelX = pos.x +. width +. 1.;

      // ~allowPast=false - should be clamped to last byte
      let bytePosition =
        Editor.Slow.pixelPositionToBytePosition(
          ~allowPast=false,
          ~pixelX,
          ~pixelY,
          editor,
        );

      expect.equal(
        bytePosition,
        BytePosition.{
          line: LineNumber.ofZeroBased(0),
          byte: ByteIndex.ofInt(2),
        },
      );

      // ~allowPast=true - should be lastByte + 1
      let bytePosition =
        Editor.Slow.pixelPositionToBytePosition(
          ~allowPast=true,
          ~pixelX,
          ~pixelY,
          editor,
        );

      expect.equal(
        bytePosition,
        BytePosition.{
          line: LineNumber.ofZeroBased(0),
          byte: ByteIndex.ofInt(3),
        },
      );
    })
  });
});
