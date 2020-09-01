open EditorCoreTypes;
open Oni_Core;
open Feature_Editor;
open TestFramework;
module LineNumber = EditorCoreTypes.LineNumber;

open Editor;

// Create a simple font for testing that is exactly 10 pixels tall.
let font: Oni_Core.Font.t = {
  ...Oni_Core.Font.default,
  measuredHeight: 10.0,
  fontSize: 10.0,
  descenderHeight: 0.0,
};

let config = (~vimSetting as _, _) => Config.NotSet;

let ofLines = (~width=100., ~height=100., lines: array(string)) => {
  let buffer = lines |> Oni_Core.Buffer.ofLines(~font);

  let editor =
    create(~config, ~buffer=buffer |> EditorBuffer.ofBuffer, ())
    // Remove any padding so the line height is just 10.
    |> Editor.setLineHeight(~lineHeight=LineHeight.padding(0));

  (editor, buffer);
};

module DSL = {
  let line = lineNumber =>
    BytePosition.{
      byte: ByteIndex.zero,
      line: LineNumber.ofZeroBased(lineNumber),
    };
};

describe("Editor", ({describe, _}) => {
  describe("pixelPositionToBytePosition", ({test, _}) => {
    let pxToByte = Editor.Slow.pixelPositionToBytePosition(~pixelX=0.);
    test("simple test", ({expect, _}) => {
      let (editor, buffer) = ofLines([|"Line 1", "Line 2", "Line 3"|]);
    
      expect.equal(editor |> Editor.lineHeightInPixels, 10.0);
    
      // Line 1
      expect.equal(pxToByte(~buffer, ~pixelY=0., editor), DSL.line(0));
      expect.equal(pxToByte(~buffer, ~pixelY=9., editor), DSL.line(0));
    
      // Line 2
      expect.equal(pxToByte(~buffer, ~pixelY=10., editor), DSL.line(1));
      expect.equal(pxToByte(~buffer, ~pixelY=19., editor), DSL.line(1));
    
      // Line 3
      expect.equal(pxToByte(~buffer, ~pixelY=20., editor), DSL.line(2));
      expect.equal(pxToByte(~buffer, ~pixelY=40., editor), DSL.line(2));
    });
    
    test("inline element at top test", ({expect, _}) => {
      let (editor, buffer) = ofLines([|"Line 1", "Line 2", "Line 3"|]);
    
      let editor = editor
      |> Editor.addInlineElement(~uniqueId="test", ~lineNumber=LineNumber.zero,
      ~height=100);
    
      // Line 1
      expect.equal(pxToByte(~buffer, ~pixelY=0., editor), DSL.line(0));
      expect.equal(pxToByte(~buffer, ~pixelY=99., editor), DSL.line(0));
      expect.equal(pxToByte(~buffer, ~pixelY=109., editor), DSL.line(0));
    
      // Line 2
      expect.equal(pxToByte(~buffer, ~pixelY=110., editor), DSL.line(1));
    
      // Line 3
      expect.equal(pxToByte(~buffer, ~pixelY=120., editor), DSL.line(2));
      expect.equal(pxToByte(~buffer, ~pixelY=140., editor), DSL.line(2));
    });
    test("multiple inline elements test", ({expect, _}) => {
      let (editor, buffer) =
        ofLines([|"Line 1", "Line 2", "Line 3", "Line 4"|]);

      let editor =
        editor
        |> Editor.addInlineElement(
             ~uniqueId="test1",
             ~lineNumber=LineNumber.(zero + 1),
             ~height=100,
           )
        |> Editor.addInlineElement(
             ~uniqueId="test2",
             ~lineNumber=LineNumber.(zero + 2),
             ~height=100,
           );

      prerr_endline("-- STARTING!");
      // Line 1
      expect.equal(pxToByte(~buffer, ~pixelY=0., editor), DSL.line(0));
      expect.equal(pxToByte(~buffer, ~pixelY=9., editor), DSL.line(0));

      // Line 2
      expect.equal(pxToByte(~buffer, ~pixelY=10., editor), DSL.line(1));
      expect.equal(pxToByte(~buffer, ~pixelY=119., editor), DSL.line(1));

      // Line 3
      expect.equal(pxToByte(~buffer, ~pixelY=120., editor), DSL.line(2));
      expect.equal(pxToByte(~buffer, ~pixelY=229., editor), DSL.line(2));

      // Line 4
      expect.equal(pxToByte(~buffer, ~pixelY=230.1, editor), DSL.line(3));
      prerr_endline("-- FINISHED!");
      expect.equal(pxToByte(~buffer, ~pixelY=250., editor), DSL.line(3));
    });
    test("multiple inline elements test, added out of order", ({expect, _}) => {
      let (editor, buffer) =
        ofLines([|"Line 1", "Line 2", "Line 3", "Line 4"|]);

      let editor =
        editor
        |> Editor.addInlineElement(
             ~uniqueId="test2",
             ~lineNumber=LineNumber.(zero + 2),
             ~height=100,
           )
        |> Editor.addInlineElement(
             ~uniqueId="test1",
             ~lineNumber=LineNumber.(zero + 1),
             ~height=100,
           );

      // Line 1
      expect.equal(pxToByte(~buffer, ~pixelY=0., editor), DSL.line(0));
      expect.equal(pxToByte(~buffer, ~pixelY=9., editor), DSL.line(0));

      // Line 2
      expect.equal(pxToByte(~buffer, ~pixelY=10., editor), DSL.line(1));
      expect.equal(pxToByte(~buffer, ~pixelY=119., editor), DSL.line(1));

      // Line 3
      expect.equal(pxToByte(~buffer, ~pixelY=120., editor), DSL.line(2));
      expect.equal(pxToByte(~buffer, ~pixelY=229., editor), DSL.line(2));

      // Line 4
      expect.equal(pxToByte(~buffer, ~pixelY=230., editor), DSL.line(3));
      expect.equal(pxToByte(~buffer, ~pixelY=250., editor), DSL.line(3));
    });
  })
});
