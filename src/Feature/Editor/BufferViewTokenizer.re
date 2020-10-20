/*
 * BufferViewTokenizer.re
 */

open EditorCoreTypes;
open Revery;

open Oni_Core;

type tokenType =
  | Tab
  | Whitespace
  | Text;

type t = {
  tokenType,
  text: string,
  startIndex: CharacterIndex.t,
  endIndex: CharacterIndex.t,
  startByte: ByteIndex.t,
  endByte: ByteIndex.t,
  startPixel: float,
  endPixel: float,
  color: Color.t,
  backgroundColor: Color.t,
  bold: bool,
  italic: bool,
};

let space = Uchar.of_char(' ');
let tab = Uchar.of_char('\t');
let cr = Uchar.of_char('\r');
let lf = Uchar.of_char('\n');

let isWhitespace = c => {
  Uchar.equal(space, c)
  || Uchar.equal(tab, c)
  || Uchar.equal(cr, c)
  || Uchar.equal(lf, c);
};

let filterRuns = (r: Tokenizer.TextRun.t) => ZedBundled.length(r.text) != 0;

let textRunToToken = (colorizer, r: Tokenizer.TextRun.t) => {
  let {color, backgroundColor, bold, italic}: BufferLineColorizer.themedToken =
    colorizer(r.startByte);

  let firstChar = ZedBundled.get(r.text, 0);

  let tokenType =
    if (Uchar.equal(firstChar, tab)) {
      Tab;
    } else if (Uchar.equal(firstChar, space)) {
      Whitespace;
    } else {
      Text;
    };

  {
    tokenType,
    text: r.text,
    startIndex: r.startIndex,
    endIndex: r.endIndex,
    startByte: r.startByte,
    endByte: r.endByte,
    startPixel: r.startPixel,
    endPixel: r.endPixel,
    color,
    backgroundColor,
    bold,
    italic,
  };
};

let tokenize =
    (
      ~start=CharacterIndex.zero,
      ~stop,
      line,
      colorizer: ByteIndex.t => BufferLineColorizer.themedToken,
    ) => {
  let split =
      (
        ~index0 as _,
        ~byte0 as b0,
        ~char0 as c0,
        ~index1 as _,
        ~byte1 as b1,
        ~char1 as c1,
      ) => {
    let {backgroundColor: bg0, color: fg0, _}: BufferLineColorizer.themedToken =
      colorizer(b0);
    let {backgroundColor: bg1, color: fg1, _}: BufferLineColorizer.themedToken =
      colorizer(b1);

    !Color.equals(bg0, bg1)
    || !Color.equals(fg0, fg1)
    || isWhitespace(c0) != isWhitespace(c1)
    /* Always split on tabs */
    || Uchar.equal(c0, tab)
    || Uchar.equal(c1, tab);
  };

  Tokenizer.tokenize(~start, ~stop, ~f=split, line)
  |> List.filter(filterRuns)
  |> List.map(textRunToToken(colorizer));
};
