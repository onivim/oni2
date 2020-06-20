type lineWrap = {
  byte: int,
  index: int,
};

type t = BufferLine.t => list(lineWrap);

let none = _line => [{byte: 0, index: 0}];

let fixed = (~columns, bufferLine) => {
  let byteLength = BufferLine.lengthInBytes(bufferLine);

  let rec loop = (curr, currWidth, characterIndex) => {
    let byteIndex =
      BufferLine.getByteFromIndex(~index=characterIndex, bufferLine);
    if (byteIndex >= byteLength) {
      curr;
    } else {
      let (_position, width) =
        BufferLine.getPositionAndWidth(~index=characterIndex, bufferLine);

      // We haven't exceeded column size yet, so continue traversing
      if (width + currWidth <= columns) {
        loop(
          curr,
          currWidth + width,
          characterIndex + 1,
          // We have hit the column width, so drop a new line break
        );
      } else {
        let newWraps = [{byte: byteIndex, index: characterIndex}, ...curr];
        loop(newWraps, width, characterIndex + 1);
      };
    };
  };

  loop([{byte: 0, index: 0}], 0, 0) |> List.rev;
};
