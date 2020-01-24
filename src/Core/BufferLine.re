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
  char: UChar.t,
  width: int,
};

type t = {
  indentation: IndentationSettings.t,
  // [raw] is the raw string (byte array)
  raw: string,
  // [characters] is a cache of discovered characters we've found in the string so far
  characters: array(option(characterCacheInfo)),
  // nextByte is the nextByte to work from, or -1 if complete
  nextByte: int,
  // nextIndex is the nextIndex to work from
  nextIndex: int,
  // nextPosition is the graphical position (based on character width)
  nextPosition: int,
};

let make = (~indentation, raw: string) => {
  // Create a cache the size of the string - this would be the max length
  // of the UTF8 string, if it was all 1-byte unicode characters (ie, an ASCII string).
  let len = String.length(raw);
  let characters = Array.make(len, None);
  {indentation, raw, characters, nextByte: 0, nextIndex: 0, nextPosition: 0};
};

let empty = make("");

let resolveTo = (~index, cache: t) =>
  // We've already resolved to this point,
  // no work needed!
  if (index < cache.nextIndex) {
    cache;
  } else {
    // Requested an index we haven't discovered yet - so we'll need to compute up to the point

    // Create an immutable copy of the array...
    // TODO: Switch to mutable approach for perf?
    let characters = Array.copy(cache.characters);

    let i: ref(int) = ref(cache.nextIndex);
    let byte: ref(int) = ref(cache.nextByte);
    let position: ref(int) = ref(cache.nextPosition);
    while (i^ < index) {
      let (char, offset) = ZedBundled.unsafe_extract_next(cache.raw, byte^);

      // TODO: Indentation settings
      let characterWidth =
        Internal.measure(IndentationSettings.default, char);

      characters[i^] =
        Some({
          byteOffset: byte^,
          positionOffset: position^,
          char,
          width: characterWidth,
        });

      position := position^ + characterWidth;
      byte := offset;
      incr(i);
    };

    {
      indentation: cache.indentation,
      raw: cache.raw,
      characters,
      nextIndex: i^,
      nextByte: byte^,
      nextPosition: position^,
    };
  };

let lengthInBytes = ({raw, _}) => String.length(raw);

let slowLengthUtf8 = ({raw, _}) => ZedBundled.length(raw);

let raw = ({raw}) => raw;

let boundedLengthUtf8 = (~max, {raw, _}) => {
  // TODO: Make this faster...
  min(max, ZedBundled.length(raw));
};

let unsafeGetUChar = (~index, {raw, _}) => ZedBundled.get(raw, index);

let unsafeSub = (~index: int, ~length: int, {raw, _}) =>
  ZedBundled.sub(raw, index, length);
let getPositionAndWidth = (~index: int, {raw, indentation}) => {
  let x = ref(0);
  let totalOffset = ref(0);
  let len = ZedBundled.length(raw);

  let measure = Internal.measure(indentation);

  while (x^ < len && x^ < index) {
    let c = ZedBundled.get(raw, x^);
    let width = measure(c);

    totalOffset := totalOffset^ + width;

    incr(x);
  };

  let width =
    index < len && index >= 0 ? measure(ZedBundled.get(raw, index)) : 1;
  (totalOffset^, width);
};
