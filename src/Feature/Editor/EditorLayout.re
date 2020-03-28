/*
 * EditorLayout.re
 *
 * Helper utilities for laying out the editor, based on measurements
 */

open Oni_Core;

type t = {
  lineNumberWidthInPixels: float,
  bufferWidthInPixels: float,
  bufferWidthInCharacters: int,
  bufferHeightInCharacters: int,
  minimapWidthInCharacters: int,
  minimapHeightInCharacters: int,
  minimapWidthInPixels: int,
};

let getLayout =
    (
      ~showLineNumbers,
      ~maxMinimapCharacters,
      ~pixelWidth: float,
      ~pixelHeight: float,
      ~isMinimapShown: bool,
      ~characterWidth: float,
      ~characterHeight: float,
      ~bufferLineCount: int,
      (),
    ) => {
  let lineNumberWidthInPixels =
    showLineNumbers
      ? LineNumber.getLineNumberPixelWidth(
          ~lines=bufferLineCount,
          ~fontPixelWidth=characterWidth,
          (),
        )
      : 0.0;

  let minimapPadding =
    isMinimapShown ? float_of_int(Constants.minimapPadding) : 0.0;

  let availableWidthInPixels =
    pixelWidth
    -. lineNumberWidthInPixels
    -. float_of_int(Constants.scrollBarThickness)
    -. minimapPadding
    *. 2.;

  let widthInCharacters =
    if (isMinimapShown) {
      /*
       * We need to solve this equation to figure out how many characters we can show
       * in both the main buffer and minimap:
       *
       * (c * characterWidth) + (c * minimapCharacterWidth) = availableWidthInPixels
       * c * (characterWidth + minimapCharacterWidth) = availableWidthInPixels
       */
      int_of_float(
        availableWidthInPixels
        /. (characterWidth +. float_of_int(Constants.minimapCharacterWidth)),
      );
    } else {
      int_of_float(availableWidthInPixels /. characterWidth);
    };

  let minimapWidthInCharacters =
    if (isMinimapShown) {
      maxMinimapCharacters > widthInCharacters
        ? widthInCharacters : maxMinimapCharacters;
    } else {
      0;
    };

  let minimapWidthInPixels =
    Constants.minimapCharacterWidth * minimapWidthInCharacters;

  /* Recalculate available buffer width - might be extra room if minimap is truncated! */
  let availableBufferWidth =
    availableWidthInPixels -. float_of_int(minimapWidthInPixels);

  let bufferWidthInCharacters =
    int_of_float(availableBufferWidth /. characterWidth);

  let bufferWidthInPixels =
    characterWidth *. float_of_int(bufferWidthInCharacters);

  let bufferHeightInCharacters = int_of_float(pixelHeight /. characterHeight);
  let minimapHeightInCharacters =
    isMinimapShown
      ? int_of_float(
          pixelHeight
          /. float_of_int(
               Constants.minimapCharacterHeight + Constants.minimapLineSpacing,
             ),
        )
      : 0;

  let leftOverWidth =
    pixelWidth
    -. bufferWidthInPixels
    -. float_of_int(minimapWidthInPixels)
    -. lineNumberWidthInPixels
    -. float_of_int(Constants.scrollBarThickness)
    -. minimapPadding
    *. 2.;

  {
    lineNumberWidthInPixels,
    minimapWidthInPixels,
    bufferWidthInPixels: bufferWidthInPixels +. leftOverWidth,
    bufferWidthInCharacters,
    minimapWidthInCharacters,
    bufferHeightInCharacters,
    minimapHeightInCharacters,
  };
};
