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
      ~maxMinimapCharacters=120,
      ~pixelWidth: float,
      ~pixelHeight: float,
      ~isMinimapShown: bool,
      ~characterWidth: float,
      ~characterHeight: float,
      ~bufferLineCount: int,
      (),
    ) => {
  let lineNumberWidthInPixels =
    LineNumber.getLineNumberPixelWidth(
      ~lines=bufferLineCount,
      ~fontPixelWidth=characterWidth,
      (),
    );

  let availableWidthInPixels =
    pixelWidth
    -. lineNumberWidthInPixels
    -. float_of_int(Constants.default.scrollBarThickness)
    -. float_of_int(Constants.default.minimapPadding)
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
        /. (
          characterWidth
          +. float_of_int(Constants.default.minimapCharacterWidth)
        ),
      );
    } else {
      int_of_float(availableWidthInPixels /. characterWidth);
    };

  let minimapWidthInCharacters =
    maxMinimapCharacters > widthInCharacters
      ? widthInCharacters : maxMinimapCharacters;
  let minimapWidthInPixels =
    Constants.default.minimapCharacterWidth * minimapWidthInCharacters;

  /* Recalculate available buffer width - might be extra room if minimap is truncated! */
  let availableBufferWidth =
    availableWidthInPixels -. float_of_int(minimapWidthInPixels);

  let bufferWidthInCharacters =
    int_of_float(availableBufferWidth /. characterWidth);

  let bufferWidthInPixels =
    characterWidth *. float_of_int(bufferWidthInCharacters);

  let bufferHeightInCharacters = int_of_float(pixelHeight /. characterHeight);
  let minimapHeightInCharacters =
    int_of_float(
      pixelHeight
      /. float_of_int(
           Constants.default.minimapCharacterHeight
           + Constants.default.minimapLineSpacing,
         ),
    );

  let leftOverWidth =
    pixelWidth
    -. bufferWidthInPixels
    -. float_of_int(minimapWidthInPixels)
    -. lineNumberWidthInPixels
    -. float_of_int(Constants.default.scrollBarThickness)
    -. float_of_int(Constants.default.minimapPadding)
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
