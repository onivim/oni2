/*
 * EditorLayout.re
 *
 * Helper utilities for laying out the editor, based on measurements
 */

open Oni_Core;

type t = {
  lineNumberWidthInPixels: int,
  minimapWidthInPixels: int,
  bufferWidthInPixels: int,
  characterWidth: int,
  bufferHeightInCharacters: int,
  minimapHeightInCharacters: int,
}

let getLayout = (~pixelWidth: int, ~pixelHeight: int, ~isMinimapShown: bool, ~characterWidth: int, ~characterHeight: int, ~bufferLineCount: int, ()) => {
    
   let lineNumberWidthInPixels = LineNumber.getLineNumberPixelWidth(~lines=bufferLineCount, ~fontPixelWidth=characterWidth, ()); 

   let availableWidthInPixels = lineNumberWidthInPixels - Constants.scrollBarThickness - Constants.minimapPadding * 2;

   /*
    * We need to solve this equation to figure out how many characters we can show
    * in both the main buffer and minimap:
    *
    * (c * characterWidth) + (c * minimapCharacterWidth) = availableWidthInPixels
    * c * (characterWidth + minimapCharacterWidth) = availableWidthInPixels
    */
   let widthInCharacters = availableWidthInPixels / (characterWidth + Constants.minimapCharacterWidth);

   let bufferHeightInCharacters = pixelHeight / characterHeight;
   let minimapHeightInCharacters = pixelHeight / minimapHeightInCharacters;
}

