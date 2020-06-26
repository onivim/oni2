/*
 * Buffer.re
 *
 * In-memory text buffer representation
 */
exception OutOfBounds;

let _space = Uchar.of_char(' ');
let tab = Uchar.of_char('\t');
let _cr = Uchar.of_char('\r');
let _lf = Uchar.of_char('\n');

type characterCacheInfo = {
  byteOffset: int,
  positionOffset: int,
  uchar: Uchar.t,
  width: int,
};

// We use these 'empty' values to reduce allocations in the normal workflow
// of creating buffer lines. We defer actually creating the arrays / caches
// until we need them.

// This is important for performance for loading large files, and prevents
// us from needing to allocate arrays for every line upon load of the buffer.

// These 'placeholder' values are treated like a [None] - we can use a reference
// equality check against these to see if we've allocated already.
let emptyCharacterMap: array(option(characterCacheInfo)) = [||];
let emptyByteIndexMap: array(option(int)) = [||];

type t = {
  indentation: IndentationSettings.t,
  // [raw] is the raw string (byte array)
  raw: string,
  // [characters] is a cache of discovered characters we've found in the string so far
  mutable characters: array(option(characterCacheInfo)),
  // [byteIndexMap] is a cache of byte -> index
  mutable byteIndexMap: array(option(int)),
  // nextByte is the nextByte to work from, or -1 if complete
  mutable nextByte: int,
  // nextIndex is the nextIndex to work from
  mutable nextIndex: int,
  // nextPosition is the graphical position (based on character width)
  mutable nextPosition: int,
};

module Internal = {
  let measure = (indentationSettings: IndentationSettings.t, c) =>
    if (Uchar.equal(c, tab)) {
      indentationSettings.tabSize;
    } else {
      Uucp.Break.tty_width_hint(c);
    };

  let resolveTo = (~index, cache: t) => {
    // First, allocate our cache, if necessary
    if (cache.characters === emptyCharacterMap) {
      cache.characters = Array.make(String.length(cache.raw), None);
    };

    if (cache.byteIndexMap === emptyByteIndexMap) {
      cache.byteIndexMap = Array.make(String.length(cache.raw), None);
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

        let idx = i^;
        let byteOffset = byte^;
        cache.characters[idx] =
          Some({
            byteOffset,
            positionOffset: position^,
            uchar,
            width: characterWidth,
          });

        cache.byteIndexMap[byteOffset] = Some(idx);

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
  {
    // Create a cache the size of the string - this would be the max length
    // of the UTF8 string, if it was all 1-byte unicode characters (ie, an ASCII string).

    indentation,
    raw,
    characters: emptyCharacterMap,
    byteIndexMap: emptyByteIndexMap,
    nextByte: 0,
    nextIndex: 0,
    nextPosition: 0,
  };
};

let empty = make(~indentation=IndentationSettings.default, "");

let lengthInBytes = ({raw, _}) => String.length(raw);

let lengthSlow = ({raw, _}) => ZedBundled.length(raw);

let raw = ({raw, _}) => raw;

let lengthBounded = (~max, bufferLine) => {
  Internal.resolveTo(~index=max, bufferLine);
  min(bufferLine.nextIndex, max);
};

let getIndex = (~byte, bufferLine) => {
  Internal.resolveTo(~index=byte, bufferLine);

  // In the case where we are looking at an 'intermediate' byte,
  // work backwards to the previous matching index.
  let rec loop = idx =>
    if (idx <= 0) {
      0;
    } else {
      switch (bufferLine.byteIndexMap[idx]) {
      | Some(v) => v
      | None => loop(idx - 1)
      };
    };

  // If we're asking for a byte past the length of the string,
  // return the index that would be past the last index. The reason
  // we handle this case - as opposed throw - is to handle the
  // case where a cursor position is past the end of the current string.
  if (byte >= String.length(bufferLine.raw)) {
    bufferLine.nextIndex;
  } else {
    loop(byte);
  };
};

let getUcharExn = (~index, bufferLine) => {
  Internal.resolveTo(~index, bufferLine);
  let characters = bufferLine.characters;
  switch (characters[index]) {
  | Some({uchar, _}) => uchar
  | None => raise(OutOfBounds)
  };
};

let getByteFromIndex = (~index, bufferLine) => {
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
  let startOffset = getByteFromIndex(~index, bufferLine);
  let endOffset = getByteFromIndex(~index=index + length, bufferLine);
  String.sub(bufferLine.raw, startOffset, endOffset - startOffset);
};

let getPositionAndWidth = (~index: int, bufferLine: t) => {
  Internal.resolveTo(~index, bufferLine);
  let characters = bufferLine.characters;
  let len = Array.length(characters);

  if (index < 0 || index >= len || len == 0) {
    (bufferLine.nextPosition, 1);
  } else {
    switch (characters[index]) {
    | Some({positionOffset, width, _}) => (positionOffset, width)
    | None =>
      switch (characters[bufferLine.nextIndex - 1]) {
      | Some({positionOffset, width, _}) => (positionOffset + width, 1)
      | None => (0, 1)
      }
    };
  };
};

module Slow = {
  let getByteFromPosition = (~position, bufferLine) => {
    let length = lengthInBytes(bufferLine);
    let rec loop = byteIndex =>
      if (byteIndex >= length) {
        length - 1;
      } else {
        let index = getIndex(~byte=byteIndex, bufferLine);
        let (characterPosition, width) =
          getPositionAndWidth(~index, bufferLine);

        if (position >= characterPosition
            && position < characterPosition
            + width) {
          index;
        } else {
          loop(byteIndex + 1);
        };
      };

    loop(0);
  };
};
