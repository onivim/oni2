open EditorCoreTypes;

type lineWrap = {
  byte: ByteIndex.t,
  character: CharacterIndex.t,
};

type t = BufferLine.t => array(lineWrap);

let none = _line => [|
  {byte: ByteIndex.zero, character: CharacterIndex.zero},
|];

let fixed = (~simpleMeasurement=true, ~pixels, bufferLine) => {
  let byteLength = BufferLine.lengthInBytes(bufferLine);
  let raw = BufferLine.raw(bufferLine);

  let columnsInPixels = pixels;

  let singleWidthCharacter =
    BufferLine.measure(bufferLine, Uchar.of_char('W'));
  let doubleWidthCharacter = singleWidthCharacter *. 2.;
  let measure =
    if (simpleMeasurement) {
      uchar =>
        if (Uucp.Break.tty_width_hint(uchar) > 1) {
          doubleWidthCharacter;
        } else {
          singleWidthCharacter;
        };
    } else {
      BufferLine.measure(bufferLine);
    };

  let rec loop = (acc, currentByte, currentCharacter, currentWidth) =>
    if (currentByte >= byteLength) {
      acc;
    } else {
      let (uchar, offset) = Zed_utf8.unsafe_extract_next(raw, currentByte);
      let width = measure(uchar);

      // We haven't exceeded column size yet, so continue traversing
      if (width +. currentWidth <= columnsInPixels) {
        loop(acc, offset, currentCharacter + 1, currentWidth +. width);
      } else {
        // We have hit the column width, so drop a new line break
        let newWraps = [
          {
            byte: ByteIndex.ofInt(currentByte),
            character: CharacterIndex.ofInt(currentCharacter),
          },
          ...acc,
        ];
        loop(newWraps, offset, currentCharacter + 1, width);
      };
    };

  loop([{byte: ByteIndex.zero, character: CharacterIndex.zero}], 0, 0, 0.)
  |> List.rev
  |> Array.of_list;
};
