open EditorCoreTypes;

type lineWrap = {
  byte: ByteIndex.t,
  character: CharacterIndex.t,
};

type t = BufferLine.t => (array(lineWrap), float);

module Constants = {
  let noWrap = [|{byte: ByteIndex.zero, character: CharacterIndex.zero}|];
};

module Internal = {
  let createMeasureFunction = (~simpleMeasurement, bufferLine) => {
    let singleWidthCharacter =
      BufferLine.measure(bufferLine, Uchar.of_char('W'));
    let doubleWidthCharacter = singleWidthCharacter *. 2.;
    let tabChar = Uchar.of_char('\t');
    let tabSize = BufferLine.measure(bufferLine, tabChar);
    if (simpleMeasurement) {
      uchar =>
        if (Uucp.Break.tty_width_hint(uchar) > 1) {
          doubleWidthCharacter;
        } else if (Uchar.equal(uchar, tabChar)) {
          tabSize;
        } else {
          singleWidthCharacter;
        };
    } else {
      BufferLine.measure(bufferLine);
    };
  };
};

let none = line => {
  // Even though there are no wraps, we have to figure out
  // the total pixel width of the line
  let measure = Internal.createMeasureFunction(~simpleMeasurement=true, line);
  let byteLength = BufferLine.lengthInBytes(line);
  let raw = BufferLine.raw(line);

  let rec loop = (acc, currentByte) =>
    if (currentByte >= byteLength) {
      acc;
    } else {
      let (uchar, offset) = Zed_utf8.unsafe_extract_next(raw, currentByte);
      let width = measure(uchar);
      loop(acc +. width, offset);
    };

  let totalWidth = loop(0., 0);

  (Constants.noWrap, totalWidth);
};

let fixed = (~simpleMeasurement=true, ~pixels, bufferLine) => {
  let byteLength = BufferLine.lengthInBytes(bufferLine);
  let raw = BufferLine.raw(bufferLine);

  let columnsInPixels = pixels;

  let measure =
    Internal.createMeasureFunction(~simpleMeasurement, bufferLine);

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

  let wraps =
    loop([{byte: ByteIndex.zero, character: CharacterIndex.zero}], 0, 0, 0.)
    |> List.rev
    |> Array.of_list;

  (wraps, pixels);
};
