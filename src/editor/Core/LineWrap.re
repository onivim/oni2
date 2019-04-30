/**
 * LineWrap.re
 */
open Types;
open WrapMode;

type wrap = {
  /*
   * The UTF-8 index of the start of a wrapped (virtual) line
   */
  index: int,
  /*
   * The length of the virtual line
   */
  length: int,
};

type t = array(wrap);

let empty = [|{index: 0, length: 0}|];

let create = (s, wrapMode) => {
  let len = Zed_utf8.length(s);

  switch (wrapMode) {
  | NoWrap => [|{index: 0, length: len}|]
  | WrapColumn(width) =>
    let idx = ref(0);
    let wraps = ref([]);

    /*
     	Very naive logic for line wrapping!

     	Should take into account:
     	- Multi-width characters (logic assumes every character is 1 width)
     	- Different wrapping strategies (wrap _words_)
     	- Matching indent of parent line
     	- Showing indent character
     */
    while (idx^ < len) {
      let i = idx^;
      let segment = min(width, len - i);
      let wrap = {index: i, length: segment};
      wraps := [wrap, ...wraps^];
      idx := i + segment;
    };

    let ret = wraps^ |> List.rev |> Array.of_list;

    switch (Array.length(ret)) {
    | 0 => empty
    | _ => ret
    };
  };
};

let count = v => Array.length(v);

let toVirtualPosition = (character, v) => {
  let c = Index.toInt0(character);
  let len = Array.length(v);

  let idx = ref(0);
  let curr = ref(c);
  let found = ref(false);

  while (idx^ < len && ! found^) {
    let i = idx^;

    let currentLine = v[i];

    if (curr^ >= currentLine.length) {
      curr := curr^ - currentLine.length;
    } else {
      found := true;
    };

    incr(idx);
  };

  Position.ofInt0(idx^ - 1, curr^);
};

let getOffsets = (idx, v) => {
  let idx = Index.toInt0(idx);
  let {index, length} = v[idx];
  (index, length);
};
