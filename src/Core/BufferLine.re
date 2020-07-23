/*
 * Buffer.re
 *
 * In-memory text buffer representation
 */
exception OutOfBounds;

module Log = (val Timber.Log.withNamespace("Oni2.Core.BufferLine"));

let _space = Uchar.of_char(' ');
let tab = Uchar.of_char('\t');
let _cr = Uchar.of_char('\r');
let _lf = Uchar.of_char('\n');

type characterCacheInfo = {
  byteOffset: int,
  positionCharacterOffset: int,
  positionPixelOffset: float,
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
  // [glyphStrings] is a list of typefaces and strings from shaping
  glyphStrings: list((Skia.Typeface.t, string)),
  font: Font.t,
  // [characters] is a cache of discovered characters we've found in the string so far
  mutable characters: array(option(characterCacheInfo)),
  // [byteIndexMap] is a cache of byte -> index
  mutable byteIndexMap: array(option(int)),
  // nextByte is the nextByte to work from, or -1 if complete
  mutable nextByte: int,
  // nextIndex is the nextIndex to work from
  mutable nextIndex: int,
  mutable nextGlyphStringIndex: int,
  mutable nextGlyphStringByte: int,
  // nextCharacterPosition is the graphical position (based on character width)
  mutable nextCharacterPosition: int,
  // nextPixelPosition is the graphical position (based on pixels)
  mutable nextPixelPosition: float,
};

module Internal = {
  module SkiaTypefaceUcharHashable = {
    type t = (Skia.Typeface.t, Uchar.t);

    let equal = ((tf1, uc1), (tf2, uc2)) =>
      Skia.Typeface.getUniqueID(tf1) == Skia.Typeface.getUniqueID(tf2)
      && Uchar.equal(uc1, uc2);

    let hash = ((tf, uc)) =>
      Int32.to_int(Skia.Typeface.getUniqueID(tf)) + Uchar.hash(uc);
  };

  module MeasureResult = {
    // The measure result is of the form (characterWidth, pixelWidth)
    type t = (int, float);

    let weight = _ => 1;
  };

  module MeasurementsCache =
    Lru.M.Make(SkiaTypefaceUcharHashable, MeasureResult);

  let measurementsCache =
    MeasurementsCache.create(~initialSize=1024, 128 * 1024);

  let paint = Skia.Paint.make();
  Skia.Paint.setTextEncoding(paint, GlyphId);

  let measure =
      (
        ~typeface: Skia.Typeface.t,
        ~indentationSettings: IndentationSettings.t,
        substr,
        uchar,
      ) =>
    switch (MeasurementsCache.find((typeface, uchar), measurementsCache)) {
    | Some(result) =>
      MeasurementsCache.promote((typeface, uchar), measurementsCache);
      Log.debugf(m =>
        m(
          "MeasurementCache : Hit! Typeface : %s, Uchar: %s (%d)",
          Skia.Typeface.getFamilyName(typeface),
          Zed_utf8.singleton(uchar),
          Uchar.to_int(uchar),
        )
      );
      result;
    | None =>
      Log.debugf(m =>
        m(
          "MeasurementCache : Miss! Typeface : %s, Uchar: %s (%d)",
          Skia.Typeface.getFamilyName(typeface),
          Zed_utf8.singleton(uchar),
          Uchar.to_int(uchar),
        )
      );
      let characterWidth =
        if (Uchar.equal(uchar, tab)) {
          indentationSettings.tabSize;
        } else {
          Uucp.Break.tty_width_hint(uchar);
        };
      Skia.Paint.setTypeface(paint, typeface);
      let pixelWidth = Skia.Paint.measureText(paint, substr, None);
      let result = (characterWidth, pixelWidth);
      MeasurementsCache.add((typeface, uchar), result, measurementsCache);
      MeasurementsCache.trim(measurementsCache);
      result;
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
      let characterPosition: ref(int) = ref(cache.nextCharacterPosition);
      let pixelPosition: ref(float) = ref(cache.nextPixelPosition);

      let glyphStringIndex: ref(int) = ref(cache.nextGlyphStringIndex);
      let glyphStringByte: ref(int) = ref(cache.nextGlyphStringByte);

      while (i^ <= index && byte^ < len) {
        let (uchar, offset) =
          ZedBundled.unsafe_extract_next(cache.raw, byte^);

        let (skiaFace, glyphStr) =
          glyphStringIndex^ |> List.nth(cache.glyphStrings);
        let glyphSubstr = String.sub(glyphStr, glyphStringByte^, 2);

        let glyphNumber = {
          let lowBit = glyphSubstr.[0] |> Char.code;
          let highBit = glyphSubstr.[1] |> Char.code;
          highBit lsl 8 lor lowBit;
        };

        let (characterWidth, pixelWidth) =
          measure(
            ~typeface=skiaFace,
            ~indentationSettings=cache.indentation,
            glyphSubstr,
            uchar,
          );

        Log.debugf(m =>
          m(
            "resolveTo loop: glyphStringIndex : %d, glyphStringByte : %d, pixelPosition : %f, glyphNumber : %d",
            glyphStringIndex^,
            glyphStringByte^,
            pixelPosition^,
            glyphNumber,
          )
        );

        let idx = i^;
        let byteOffset = byte^;
        cache.characters[idx] =
          Some({
            byteOffset,
            positionCharacterOffset: characterPosition^,
            positionPixelOffset: pixelPosition^,
            uchar,
            width: characterWidth,
          });

        cache.byteIndexMap[byteOffset] = Some(idx);

        characterPosition := characterPosition^ + characterWidth;
        pixelPosition := pixelPosition^ +. pixelWidth;
        byte := offset;
        if (glyphStringByte^ + 2 >= String.length(glyphStr)) {
          glyphStringByte := 0;
          glyphStringIndex := glyphStringIndex^ + 1;
          Log.debug("Reached end of current glyphString");
        } else {
          glyphStringByte := glyphStringByte^ + 2;
          Log.debug("Continuing on current glyphString");
        };
        incr(i);
      };

      cache.nextIndex = i^;
      cache.nextByte = byte^;
      cache.nextCharacterPosition = characterPosition^;
      cache.nextPixelPosition = pixelPosition^;
      cache.nextGlyphStringIndex = glyphStringIndex^;
      cache.nextGlyphStringByte = glyphStringByte^;
    };
  };
};

let make = (~indentation, ~font: Font.t=Font.default, raw: string) => {
  let Font.{fontFamily, features, smoothing, fontSize, _} = font;

  let loadedFont =
    fontFamily
    |> Revery.Font.Family.toSkia(Revery.Font.Weight.Normal)
    |> Revery.Font.load
    |> Result.get_ok;

  let glyphStrings =
    Revery.Font.shape(~features, loadedFont, raw).glyphStrings;
  {
    // Create a cache the size of the string - this would be the max length
    // of the UTF8 string, if it was all 1-byte unicode characters (ie, an ASCII string).

    indentation,
    raw,
    glyphStrings,
    font,
    characters: emptyCharacterMap,
    byteIndexMap: emptyByteIndexMap,
    nextByte: 0,
    nextIndex: 0,
    nextGlyphStringIndex: 0,
    nextGlyphStringByte: 0,
    nextCharacterPosition: 0,
    nextPixelPosition: 0.,
  };
};

let empty = (~font=Font.default, ()) =>
  make(~indentation=IndentationSettings.default, ~font, "");

let lengthInBytes = ({raw, _}) => String.length(raw);

let lengthSlow = ({raw, _}) => ZedBundled.length(raw);

let raw = ({raw, _}) => raw;

let font = ({font, _}) => font;

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
  let len = Array.length(characters);
  if (index < 0) {
    0;
  } else if (index >= len) {
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

let getCharacterPositionAndWidth = (~index: int, bufferLine: t) => {
  Internal.resolveTo(~index, bufferLine);
  let characters = bufferLine.characters;
  let len = Array.length(characters);

  if (index < 0 || index >= len || len == 0) {
    (bufferLine.nextCharacterPosition, 1);
  } else {
    switch (characters[index]) {
    | Some({positionCharacterOffset, width, _}) => (
        positionCharacterOffset,
        width,
      )
    | None =>
      switch (characters[bufferLine.nextIndex - 1]) {
      | Some({positionCharacterOffset, width, _}) => (
          positionCharacterOffset + width,
          1,
        )
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
          getCharacterPositionAndWidth(~index, bufferLine);

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
