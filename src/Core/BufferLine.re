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
type t = {
  raw: string,
  indentation: IndentationSettings.t,
};

let make = (~indentation, str) => {indentation, raw: str};

let lengthInBytes = ({raw, _}) => String.length(raw);

let slowLengthUtf8 = ({raw, _}) => ZedBundled.length(raw);
let raw = ({raw, _}) => raw;

// TODO: Make this faster...
let boundedLengthUtf8 = (~max, {raw, _}) =>
  min(max, ZedBundled.length(raw));

// TODO: Make this faster...
let unsafeGetUChar = (~index, {raw, _}) => ZedBundled.get(raw, index);

let unsafeSub = (~index: int, ~length: int, {raw, _}) =>
  ZedBundled.sub(raw, index, length);

module Internal = {
  let measure = (indentationSettings: IndentationSettings.t, c) =>
    if (UChar.eq(c, tab)) {
      indentationSettings.tabSize;
    } else {
      1;
      // TODO: Integrate charWidth / wcwidth
    };
};

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
