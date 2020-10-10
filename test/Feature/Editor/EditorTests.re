open Oni_Core;
open EditorCoreTypes;
open TestFramework;

module Buffer = Oni_Core.Buffer;
module Editor = Feature_Editor.Editor;
module EditorBuffer = Feature_Editor.EditorBuffer;

open Oni_Core_Test.Helpers;

describe("Editor", ({describe, _}) => {
  let config = (~vimSetting as _, _key) => Config.NotSet;

  let create = lines => {
    let buffer = lines |> Buffer.ofLines(~font=Font.default());

    let editorBuffer = buffer |> EditorBuffer.ofBuffer;

    (Editor.create(~config, ~buffer=editorBuffer, ()), buffer);
  };

  describe("pixelPositionToBytePosition", ({test, _}) => {
    test("~allowPast boundary cases", ({expect, _}) => {
      let (editor, buffer) = create([|"abc"|]);

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
          ~buffer,
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
          ~buffer,
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
