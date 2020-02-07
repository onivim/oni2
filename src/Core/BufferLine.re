/*
 * Buffer.re
 *
 * In-memory text buffer representation
 */
exception OutOfBounds;

open CamomileBundled.Camomile;
let _space = UChar.of_char(' ');
let tab = UChar.of_char('\t');
let _cr = UChar.of_char('\r');
let _lf = UChar.of_char('\n');

type characterCacheInfo = {
  byteOffset: int,
  positionOffset: int,
  uchar: UChar.t,
  width: int,
};

let emptyArray = Array.make(0, None);

type t = {
  indentation: IndentationSettings.t,
  // [raw] is the raw string (byte array)
  raw: string,
  // [characters] is a cache of discovered characters we've found in the string so far
  mutable characters: array(option(characterCacheInfo)),
  // nextByte is the nextByte to work from, or -1 if complete
  mutable nextByte: int,
  // nextIndex is the nextIndex to work from
  mutable nextIndex: int,
  // nextPosition is the graphical position (based on character width)
  mutable nextPosition: int,
};

module Internal = {
  let measure = (indentationSettings: IndentationSettings.t, c) =>
    if (UChar.eq(c, tab)) {
      indentationSettings.tabSize;
    } else {
      1;
      // TODO: Integrate charWidth / wcwidth
    };

  let resolveTo = (~index, cache: t) => {
    // First, allocate our cache, if necessary
    if (cache.characters === emptyArray) {
      cache.characters = Array.make(String.length(cache.raw), None);
    };

    // We've already resolved to this point,
    // no work needed!
    if (index < cache.nextIndex) {
      ();
    } else {
      // Requested an index we haven't discovered yet - so we'll need to compute up to the point
      let len = String.length(cache.raw);

      let i: ref(int) = ref(cache.nextIndex);
      let byte: ref(int) = ref(cache.nextByte);
      let position: ref(int) = ref(cache.nextPosition);
      while (i^ <= index && byte^ < len) {
        let (uchar, offset) =
          ZedBundled.unsafe_extract_next(cache.raw, byte^);

        let characterWidth = measure(cache.indentation, uchar);

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

      cache.nextIndex = i^;
      cache.nextByte = byte^;
      cache.nextPosition = position^;
    };
  };
};

let make = (~indentation, raw: string) => {
  // Create a cache the size of the string - this would be the max length
  // of the UTF8 string, if it was all 1-byte unicode characters (ie, an ASCII string).
  let characters = emptyArray;
  {indentation, raw, characters, nextByte: 0, nextIndex: 0, nextPosition: 0};
};

let empty = make(~indentation=IndentationSettings.default, "");

let lengthInBytes = ({raw, _}) => String.length(raw);

let lengthSlow = ({raw, _}) => ZedBundled.length(raw);

let raw = ({raw, _}) => raw;

let lengthBounded = (~max, bufferLine) => {
  Internal.resolveTo(~index=max, bufferLine);
  min(bufferLine.nextIndex, max);
};

let getUCharExn = (~index, bufferLine) => {
  Internal.resolveTo(~index, bufferLine);
  let characters = bufferLine.characters;
  switch (characters[index]) {
  | Some({uchar, _}) => uchar
  | None => raise(OutOfBounds)
  };
};

let getByteOffset = (~index, bufferLine) => {
  Internal.resolveTo(~index, bufferLine);
  let rawLength = String.length(bufferLine.raw);
  let characters = bufferLine.characters;
  if (index >= Array.length(characters)) {
    rawLength;
  } else {
    switch (characters[index]) {
    | Some({byteOffset, _}) => byteOffset
    | None => rawLength
    };
  };
};

let subExn = (~index: int, ~length: int, bufferLine) => {
  let startOffset = getByteOffset(~index, bufferLine);
  let endOffset = getByteOffset(~index=index + length, bufferLine);
  String.sub(bufferLine.raw, startOffset, endOffset - startOffset);
};

let getPositionAndWidth = (~index: int, bufferLine: t) => {
  Internal.resolveTo(~index, bufferLine);
  let characters = bufferLine.characters;

  if (index >= Array.length(characters)) {
    (bufferLine.nextPosition, 1);
  } else {
    switch (characters[index]) {
    | Some({positionOffset, width, _}) => (positionOffset, width)
    | None => (0, 1)
    };
  };
};
