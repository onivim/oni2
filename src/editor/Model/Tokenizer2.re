/*
 * Tokenizer2.re
 */

open Oni_Core.Types;

open CamomileLibrary;

module TextRun = {
  type t = {
    text: string,
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
      (~text, ~startIndex, ~endIndex, ~startPosition, ~endPosition, ()) => {
    text,
    startIndex,
    endIndex,
    startPosition,
    endPosition,
  };
};

type splitFunc = (int, UChar.t, int, UChar.t) => bool;

type measureFunc = UChar.t => int;

let _getNextBreak = (s: string, start: int, max: int, f: splitFunc) => {
  let pos = ref(start);
  let found = ref(false);

  while (pos^ < max - 1 && ! found^) {
    let firstPos = pos^;
    let secondPos = pos^ + 1;
    let char = Zed_utf8.get(s, firstPos);
    let nextChar = Zed_utf8.get(s, secondPos);

    if (f(firstPos, char, secondPos, nextChar)) {
      found := true;
    };

    if (! found^) {
      incr(pos);
    };
  };

  pos^;
};

let defaultMeasure: measureFunc = _ => 1;

let tokenize = (~f: splitFunc, ~measure=defaultMeasure, s: string) => {
  let startIndex = 0;
  let maxIndex = Zed_utf8.length(s);
  let idx = ref(startIndex);
  let tokens: ref(list(TextRun.t)) = ref([]);

  let offset = ref(0);

  while (idx^ < maxIndex) {
    let startToken = idx^;
    let startOffset = offset^;
    let endToken = _getNextBreak(s, startToken, maxIndex, f) + 1;

    let text = Zed_utf8.sub(s, startToken, endToken - startToken);
    let endOffset =
      startOffset
      + Zed_utf8.fold((char, prev) => prev + measure(char), text, 0);

    let textRun =
      TextRun.create(
        ~text,
        ~startIndex=ZeroBasedIndex(startToken),
        ~endIndex=ZeroBasedIndex(endToken),
        ~startPosition=ZeroBasedIndex(startOffset),
        ~endPosition=ZeroBasedIndex(endOffset),
        (),
      );

    tokens := [textRun, ...tokens^];
    idx := endToken;
    offset := endOffset;
  };

  tokens^ |> List.rev;
};
