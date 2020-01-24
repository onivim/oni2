/*
 * Buffer.re
 *
 * In-memory text buffer representation
 */
module Option = Utility.Option;

open CamomileBundled.Camomile;
let _space = UChar.of_char(' ');
let tab = UChar.of_char('\t');
let _cr = UChar.of_char('\r');
let _lf = UChar.of_char('\n');

module Internal = {
  let measure = (indentationSettings: IndentationSettings.t, c) =>
    if (UChar.eq(c, tab)) {
      indentationSettings.tabSize;
    } else {
      1;
      // TODO: Integrate charWidth / wcwidth
    };
};

type characterCacheInfo = {
  byteOffset: int,
  positionOffset: int,
  uchar: UChar.t,
  width: int,
};

type t = {
  indentation: IndentationSettings.t,
  // [raw] is the raw string (byte array)
  raw: string,
  // [characters] is a cache of discovered characters we've found in the string so far
  characters: array(option(characterCacheInfo)),
  // nextByte is the nextByte to work from, or -1 if complete
  nextByte: ref(int),
  // nextIndex is the nextIndex to work from
  nextIndex: ref(int),
  // nextPosition is the graphical position (based on character width)
  nextPosition: ref(int),
};

let make = (~indentation, raw: string) => {
  // Create a cache the size of the string - this would be the max length
  // of the UTF8 string, if it was all 1-byte unicode characters (ie, an ASCII string).
  let len = String.length(raw);
  let characters = Array.make(len, None);
  {indentation, raw, characters, nextByte: ref(0), nextIndex: ref(0), nextPosition: ref(0)};
};

let empty = make(~indentation=IndentationSettings.default, "");

let _resolveTo = (~index, cache: t) =>
  // We've already resolved to this point,
  // no work needed!
  if (index < cache.nextIndex^) {
    ();
  } else {
    // Requested an index we haven't discovered yet - so we'll need to compute up to the point
    let len = String.length(cache.raw)

    let i: ref(int) = ref(cache.nextIndex^);
    let byte: ref(int) = ref(cache.nextByte^);
    let position: ref(int) = ref(cache.nextPosition^);
    while (i^ <= index && byte^ < len) {
      let (uchar, offset) = ZedBundled.unsafe_extract_next(cache.raw, byte^);

      let characterWidth =
        Internal.measure(cache.indentation, uchar);

      cache.characters[i^] =
        Some({
          byteOffset: byte^,
          positionOffset: position^,
          uchar,
          width: characterWidth,
        });

      position := position^ + characterWidth;
      byte := offset;
      incr(i);
    };

    cache.nextIndex := i^;
    cache.nextByte := byte^;
    cache.nextPosition := position^;
  };

let lengthInBytes = ({raw, _}) => String.length(raw);

let slowLengthUtf8 = ({raw, _}) => ZedBundled.length(raw);

let raw = ({raw, _}) => raw;

let boundedLengthUtf8 = (~max, bufferLine) => {
  _resolveTo(~index=max, bufferLine)
  min(bufferLine.nextIndex^, max);
};

let unsafeGetUChar = (~index, bufferLine) => {
  _resolveTo(~index, bufferLine);
  switch(bufferLine.characters[index]) {
  | Some({ uchar, _}) => uchar
  | None => failwith("invalid index (out of bounds)")
  }
}

let getByteOffset = (~index, bufferLine) => {
  _resolveTo(~index, bufferLine);
  let rawLength = String.length(bufferLine.raw);
  if (index >= Array.length(bufferLine.characters)) {
    rawLength;
  } else {
    switch (bufferLine.characters[index]) {
    | Some({byteOffset, _}) => byteOffset
    | None => rawLength
    };
  }
}

let unsafeSub = (~index: int, ~length: int, bufferLine) => {
  let startOffset = getByteOffset(~index, bufferLine);
  let endOffset = getByteOffset(~index=index+length, bufferLine);
  String.sub(bufferLine.raw, startOffset, endOffset - startOffset);
}

let getPositionAndWidth = (~index: int, bufferLine: t) => {
  _resolveTo(~index, bufferLine);

  if (index >= Array.length(bufferLine.characters)) {
    (bufferLine.nextPosition^, 1)
  } else {
    switch (bufferLine.characters[index]) {
    | Some({positionOffset, width, _}) => (positionOffset, width)
    | None => (0, 1)
    }
  }
};
