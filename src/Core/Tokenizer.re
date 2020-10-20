/*
 * Tokenizer.re
 */

open EditorCoreTypes;

module TextRun = {
  type t = {
    text: string,
    /*
     * Bytes refer to the byte position in the parent string
     */
    startByte: ByteIndex.t,
    endByte: ByteIndex.t,
    /*
     * Character indices refer to the UTF-8 position in the parent string
     */
    startIndex: CharacterIndex.t,
    endIndex: CharacterIndex.t,
    /*
     * Pixel positions for the start / end of the token
     */
    startPixel: float,
    endPixel: float,
  };

  let create =
      (
        ~text,
        ~startByte,
        ~endByte,
        ~startIndex,
        ~endIndex,
        ~startPixel,
        ~endPixel,
        (),
      ) => {
    text,
    startByte,
    endByte,
    startIndex,
    endIndex,
    startPixel,
    endPixel,
  };
};

type splitFunc =
  (
    ~index0: CharacterIndex.t,
    ~byte0: ByteIndex.t,
    ~char0: Uchar.t,
    ~index1: CharacterIndex.t,
    ~byte1: ByteIndex.t,
    ~char1: Uchar.t
  ) =>
  bool;

module Internal = {
  let getNextBreak =
      (bufferLine: BufferLine.t, start: int, max: int, f: splitFunc) => {
    let pos = ref(start);
    let found = ref(false);

    while (pos^ < max - 1 && ! found^) {
      let index0 = CharacterIndex.ofInt(pos^);
      let index1 = CharacterIndex.ofInt(pos^ + 1);
      let char0 = BufferLine.getUcharExn(~index=index0, bufferLine);
      let char1 = BufferLine.getUcharExn(~index=index1, bufferLine);
      let byte0 = BufferLine.getByteFromIndex(~index=index0, bufferLine);
      let byte1 = BufferLine.getByteFromIndex(~index=index1, bufferLine);

      if (f(~index0, ~index1, ~char0, ~char1, ~byte0, ~byte1)) {
        found := true;
      };

      if (! found^) {
        incr(pos);
      };
    };

    pos^;
  };
};

let tokenize =
    (
      ~start=CharacterIndex.zero,
      ~stop: CharacterIndex.t,
      ~f: splitFunc,
      bufferLine: BufferLine.t,
    ) => {
  let startIndex = CharacterIndex.toInt(start);
  let endIndex = CharacterIndex.toInt(stop);
  let len = BufferLine.lengthBounded(~max=stop, bufferLine);

  if (len == 0 || startIndex >= len) {
    [];
  } else {
    let maxIndex = endIndex < 0 || endIndex > len ? len : endIndex;

    let idx = ref(startIndex);
    let tokens: ref(list(TextRun.t)) = ref([]);

    let (initialPixel, _) =
      BufferLine.getPixelPositionAndWidth(~index=start, bufferLine);
    while (idx^ < maxIndex) {
      let startToken = idx^;
      let endToken =
        Internal.getNextBreak(bufferLine, startToken, maxIndex, f) + 1;

      let text =
        BufferLine.subExn(
          ~index=CharacterIndex.ofInt(startToken),
          ~length=endToken - startToken,
          bufferLine,
        );

      let startByte =
        BufferLine.getByteFromIndex(
          ~index=CharacterIndex.ofInt(startToken),
          bufferLine,
        );
      let endByte =
        BufferLine.getByteFromIndex(
          ~index=CharacterIndex.ofInt(endToken),
          bufferLine,
        );

      let startIndex = CharacterIndex.ofInt(startToken);
      let endIndex = CharacterIndex.ofInt(endToken);

      let (tokenPixelX, _) =
        BufferLine.getPixelPositionAndWidth(~index=startIndex, bufferLine);

      let (tokenEndPixelX, _) =
        BufferLine.getPixelPositionAndWidth(~index=endIndex, bufferLine);

      let textRun =
        TextRun.create(
          ~text,
          ~startByte,
          ~endByte,
          ~startIndex,
          ~endIndex,
          ~startPixel=tokenPixelX -. initialPixel,
          ~endPixel=tokenEndPixelX -. initialPixel,
          (),
        );

      tokens := [textRun, ...tokens^];
      idx := endToken;
    };

    tokens^ |> List.rev;
  };
};
