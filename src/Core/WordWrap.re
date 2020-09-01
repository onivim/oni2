open EditorCoreTypes;

type lineWrap = {
  byte: ByteIndex.t,
  character: CharacterIndex.t,
};

type t = BufferLine.t => list(lineWrap);

let none = _line => [{byte: ByteIndex.zero, character: CharacterIndex.zero}];

let fixed = (~pixels, bufferLine) => {
  let byteLength = BufferLine.lengthInBytes(bufferLine);

  let columnsInPixels = pixels;

  let rec loop = (curr, currWidth, characterIndex) => {
    let byteIndex =
      BufferLine.getByteFromIndex(~index=characterIndex, bufferLine);
    if (ByteIndex.toInt(byteIndex) >= byteLength) {
      curr;
    } else {
      let (_position, width) =
        BufferLine.getPixelPositionAndWidth(
          ~index=characterIndex,
          bufferLine,
        );
      let nextCharacterIndex = CharacterIndex.(characterIndex + 1);

      // We haven't exceeded column size yet, so continue traversing
      if (width +. currWidth <= columnsInPixels) {
        loop(
          curr,
          currWidth +. width,
          nextCharacterIndex,
          // We have hit the column width, so drop a new line break
        );
      } else {
        let newWraps = [
          {byte: byteIndex, character: characterIndex},
          ...curr,
        ];
        loop(newWraps, width, nextCharacterIndex);
      };
    };
  };

  loop(
    [{byte: ByteIndex.zero, character: CharacterIndex.zero}],
    0.,
    CharacterIndex.zero,
  )
  |> List.rev;
};
