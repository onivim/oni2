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
  };

  let create = (~text, ~startByte, ~endByte, ~startIndex, ~endIndex, ()) => {
    text,
    startByte,
    endByte,
    startIndex,
    endIndex,
  };
};

type splitFunc =
  (
    ~index0: int,
    ~byte0: int,
    ~char0: Uchar.t,
    ~index1: int,
    ~byte1: int,
    ~char1: Uchar.t
  ) =>
  bool;

//type splitFunc = (int, Uchar.t, int, Uchar.t) => bool;

let _getNextBreak =
    (bufferLine: BufferLine.t, start: int, max: int, f: splitFunc) => {
  let pos = ref(start);
  let found = ref(false);

  while (pos^ < max - 1 && ! found^) {
    let index0 = pos^;
    let index1 = pos^ + 1;
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

let tokenize =
    (~startIndex=0, ~endIndex, ~f: splitFunc, bufferLine: BufferLine.t) => {
  let len = BufferLine.lengthBounded(~max=endIndex, bufferLine);

  if (len == 0 || startIndex >= len) {
    [];
  } else {
    let maxIndex = endIndex < 0 || endIndex > len ? len : endIndex;

    let idx = ref(startIndex);
    let tokens: ref(list(TextRun.t)) = ref([]);

    while (idx^ < maxIndex) {
      let startToken = idx^;
      let endToken = _getNextBreak(bufferLine, startToken, maxIndex, f) + 1;

      let text =
        BufferLine.subExn(
          ~index=startToken,
          ~length=endToken - startToken,
          bufferLine,
        );

      let startByte =
        BufferLine.getByteFromIndex(~index=startToken, bufferLine);
      let endByte = BufferLine.getByteFromIndex(~index=endToken, bufferLine);

      let textRun =
        TextRun.create(
          ~text,
          ~startByte,
          ~endByte,
          ~startIndex=Index.fromZeroBased(startToken),
          ~endIndex=Index.fromZeroBased(endToken),
          (),
        );

      tokens := [textRun, ...tokens^];
      idx := endToken;
    };

    tokens^ |> List.rev;
  };
};
