/*
 * EditorLayout.re
 *
 * Helper utilities for laying out the editor, based on measurements
 */

type t = {
  lineNumberWidthInPixels: int,
  minimapWidthInPixels: int,
  bufferWidthInPixels: int,
  widthInCharacters: int,
  bufferHeightInCharacters: int,
  minimapHeightInCharacters: int,
};

let getLayout =
    (
      ~pixelWidth: int,
      ~pixelHeight: int,
      ~isMinimapShown: bool,
      ~characterWidth: int,
      ~characterHeight: int,
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
    - lineNumberWidthInPixels
    - Constants.default.scrollBarThickness
    - Constants.default.minimapPadding
    * 2;

  let widthInCharacters =
    if (isMinimapShown) {
      /*
       * We need to solve this equation to figure out how many characters we can show
       * in both the main buffer and minimap:
       *
       * (c * characterWidth) + (c * minimapCharacterWidth) = availableWidthInPixels
       * c * (characterWidth + minimapCharacterWidth) = availableWidthInPixels
       */
      availableWidthInPixels
      / (characterWidth + Constants.default.minimapCharacterWidth);
    } else {
      availableWidthInPixels / characterWidth;
    };

  let bufferWidthInPixels = characterWidth * widthInCharacters;
  let minimapWidthInPixels =
    Constants.default.minimapCharacterWidth * widthInCharacters;

  let bufferHeightInCharacters = pixelHeight / characterHeight;
  let minimapHeightInCharacters =
    pixelHeight
    / (
      Constants.default.minimapCharacterHeight
      + Constants.default.minimapLineSpacing
    );

  let leftOverWidth =
    pixelWidth
    - bufferWidthInPixels
    - minimapWidthInPixels
    - lineNumberWidthInPixels
    - Constants.default.scrollBarThickness
    - Constants.default.minimapPadding
    * 2;

  {
    lineNumberWidthInPixels,
    minimapWidthInPixels,
    bufferWidthInPixels: bufferWidthInPixels + leftOverWidth,
    widthInCharacters,
    bufferHeightInCharacters,
    minimapHeightInCharacters,
  };
};
