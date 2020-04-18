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
    startByte: int,
    endByte: int,
    /*
     * Indices refer to the UTF-8 position in the parent string
     */
    startIndex: Index.t,
    endIndex: Index.t,
    /*
     * Positions refer to the 'visual' position of the string
     *
     * If there is a character, like `\t`, or characters
     * with wcwidth > 1, then this would be different
     * than startIndex / endIndex
     */
    startPosition: Index.t,
    endPosition: Index.t,
  };

  let create =
      (
        ~text,
        ~startByte,
        ~endByte,
        ~startIndex,
        ~endIndex,
        ~startPosition,
        ~endPosition,
        (),
      ) => {
    text,
    startByte,
    endByte,
    startIndex,
    endIndex,
    startPosition,
    endPosition,
  };
};

type splitFunc = (int, Uchar.t, int, Uchar.t) => bool;

let _getNextBreak =
    (bufferLine: BufferLine.t, start: int, max: int, f: splitFunc) => {
  let pos = ref(start);
  let found = ref(false);

  while (pos^ < max - 1 && ! found^) {
    let firstPos = pos^;
    let secondPos = pos^ + 1;
    let char = BufferLine.getUcharExn(~index=firstPos, bufferLine);
    let nextChar = BufferLine.getUcharExn(~index=secondPos, bufferLine);

    if (f(firstPos, char, secondPos, nextChar)) {
      found := true;
    };

    if (! found^) {
      incr(pos);
    };
  };

  pos^;
};

let tokenize =
    (~startIndex=0, ~endIndex, ~f: splitFunc, bufferLine: BufferLine.t) => {
  let len = BufferLine.lengthBounded(~max=endIndex, bufferLine);

  if (len == 0 || startIndex >= len) {
    [];
  } else {
    let maxIndex = endIndex < 0 || endIndex > len ? len : endIndex;

    let (initialOffset, _) =
      BufferLine.getPositionAndWidth(~index=startIndex, bufferLine);

    let idx = ref(startIndex);
    let tokens: ref(list(TextRun.t)) = ref([]);

    let offset = ref(initialOffset);

    while (idx^ < maxIndex) {
      let startToken = idx^;
      let startOffset = offset^;
      let endToken = _getNextBreak(bufferLine, startToken, maxIndex, f) + 1;

      let (endOffset, _) =
        BufferLine.getPositionAndWidth(~index=endToken, bufferLine);

      let text =
        BufferLine.subExn(
          ~index=startToken,
          ~length=endToken - startToken,
          bufferLine,
        );

      let startByte = BufferLine.getByte(~index=startToken, bufferLine);
      let endByte = BufferLine.getByte(~index=endToken, bufferLine);

      let textRun =
        TextRun.create(
          ~text,
          ~startByte,
          ~endByte,
          ~startIndex=Index.fromZeroBased(startToken),
          ~endIndex=Index.fromZeroBased(endToken),
          ~startPosition=Index.fromZeroBased(startOffset),
          ~endPosition=Index.fromZeroBased(endOffset),
          (),
        );

      tokens := [textRun, ...tokens^];
      idx := endToken;
      offset := endOffset;
    };

    tokens^ |> List.rev;
  };
};
