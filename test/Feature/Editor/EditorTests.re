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

  let createThreeWideWithWrapping = lines => {
    let (editor, _buffer) = create(lines);
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
  };

  let inlineElement = lineIdx =>
    Editor.makeInlineElement(
      ~key="test-inline-element",
      ~uniqueId=string_of_int(lineIdx),
      ~lineNumber=LineNumber.ofZeroBased(lineIdx),
      ~view=(~theme as _, ~uiFont as _, ()) =>
      Revery.UI.React.listToElement([])
    );

  let colorizer = (~startByte as _, _) => {
    BufferLineColorizer.{
      color: Revery.Colors.black,
      backgroundColor: Revery.Colors.white,
      italic: false,
      bold: false,
    };
  };

  let bytePos = (lnum, byteNum) =>
    BytePosition.{
      line: LineNumber.ofZeroBased(lnum),
      byte: ByteIndex.ofInt(byteNum),
    };
  describe("inline elements", ({test, _}) => {
    test("inline element at top", ({expect, _}) => {
      let (editor, _buffer) = create([|"aaa"|]);
      let inlineElement0 = inlineElement(0);
      let editor =
        editor
        |> Editor.setWrapMode(~wrapMode=WrapMode.NoWrap)
        |> Editor.setSize(~pixelWidth=500, ~pixelHeight=500)
        |> Editor.setInlineElements(
             ~key="test-inline-element",
             ~elements=[inlineElement0],
           )
        |> Editor.setInlineElementSize(
             ~key="test-inline-element",
             ~uniqueId="0",
             ~height=25,
           );

      let pixelY = Editor.viewLineToPixelY(0, editor);

      expect.float(pixelY).toBeCloseTo(25.);
    });

    test("inline element after wrapping", ({expect, _}) => {
      let editor = createThreeWideWithWrapping([|"aaaaaa", "aaaaaa"|]);
      let inlineElement1 = inlineElement(1);
      let editor =
        editor
        |> Editor.setInlineElements(
             ~key="test-inline-element",
             ~elements=[inlineElement1],
           )
        |> Editor.setInlineElementSize(
             ~key="test-inline-element",
             ~uniqueId="1",
             ~height=25,
           );

      let pixelYViewLine0 = Editor.viewLineToPixelY(0, editor);
      let pixelYViewLine1 = Editor.viewLineToPixelY(1, editor);
      let pixelYViewLine2 = Editor.viewLineToPixelY(2, editor);
      let pixelYViewLine3 = Editor.viewLineToPixelY(3, editor);

      // First two lines should not be impacted by inline element,
      // because they are before it.
      let lineHeight = Editor.lineHeightInPixels(editor);
      expect.float(pixelYViewLine0).toBeCloseTo(lineHeight *. 0.);
      expect.float(pixelYViewLine1).toBeCloseTo(lineHeight *. 1.);

      // Second two viewlines should be impacted by it, though.
      expect.float(pixelYViewLine2).toBeCloseTo(lineHeight *. 2. +. 25.);
      expect.float(pixelYViewLine3).toBeCloseTo(lineHeight *. 3. +. 25.);
    });
  });
  describe("viewTokens", ({test, _}) => {
    test("single token returned", ({expect, _}) => {
      let (editor, _buffer) = create([|"aaa"|]);
      let editor =
        editor
        |> Editor.setWrapMode(~wrapMode=WrapMode.NoWrap)
        |> Editor.setSize(~pixelWidth=500, ~pixelHeight=500);

      let viewTokens =
        Editor.viewTokens(~line=0, ~scrollX=0., ~colorizer, editor);
      expect.int(List.length(viewTokens)).toBe(1);

      let item: BufferViewTokenizer.t = List.nth(viewTokens, 0);
      expect.float(item.startPixel).toBeCloseTo(0.5);
    });
    test("single token returned, with a little bit of scroll", ({expect, _}) => {
      let (editor, _buffer) = create([|"aaa"|]);
      let editor =
        editor
        |> Editor.setWrapMode(~wrapMode=WrapMode.NoWrap)
        |> Editor.setSize(~pixelWidth=500, ~pixelHeight=500);

      let viewTokens =
        Editor.viewTokens(~line=0, ~scrollX=5., ~colorizer, editor);
      expect.int(List.length(viewTokens)).toBe(1);

      let item: BufferViewTokenizer.t = List.nth(viewTokens, 0);
      expect.float(item.startPixel).toBeCloseTo(-4.5);
    });
    test(
      "wrapped line, second viewline should not be scrolled", ({expect, _}) => {
      let editor = createThreeWideWithWrapping([|"aaaaaa"|]);

      let viewTokens =
        Editor.viewTokens(~line=1, ~scrollX=0., ~colorizer, editor);
      expect.int(List.length(viewTokens)).toBe(1);

      let item: BufferViewTokenizer.t = List.nth(viewTokens, 0);
      expect.float(item.startPixel).toBeCloseTo(0.5);
    });
    test("wrapped line, both viewlines should scrolled", ({expect, _}) => {
      let editor = createThreeWideWithWrapping([|"aaaaaa"|]);

      let viewTokens0 =
        Editor.viewTokens(~line=0, ~scrollX=5., ~colorizer, editor);
      let viewTokens1 =
        Editor.viewTokens(~line=1, ~scrollX=5., ~colorizer, editor);
      expect.int(List.length(viewTokens0)).toBe(1);
      expect.int(List.length(viewTokens1)).toBe(1);

      let item0: BufferViewTokenizer.t = List.nth(viewTokens0, 0);
      let item1: BufferViewTokenizer.t = List.nth(viewTokens1, 0);
      expect.float(item0.startPixel).toBeCloseTo(-4.5);
      expect.float(item1.startPixel).toBeCloseTo(-4.5);
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
      let editor = createThreeWideWithWrapping([|"aaaaaa"|]);
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
