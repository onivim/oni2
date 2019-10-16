/*
 * CompletionMeet.re
 *
 * Helpers for finding the completion 'meet' (the location where we should request completions),
 * based on the current line and trigger characters
 */

open Oni_Core;
open Oni_Core.Types;

open CamomileBundled.Camomile;
module Zed_utf8 = ZedBundled;

type completionMeet = {
  base: string,
  index: int,
};

type t = option(completionMeet);

let show = (v: t) => switch(v) {
| None => "(None)"
| Some(m) => Printf.sprintf("Base: |%s| Meet: %d", m.base, m.index);
}

let defaultTriggerCharacters = [UChar.of_char('.')];

let getMeetFromLine = (~triggerCharacters=defaultTriggerCharacters, ~cursor: Index.t, line: string) => {
  let cursorIdx = Index.toInt0(cursor);
  let idx = Stdlib.min(Zed_utf8.length(line) - 1, cursorIdx);
  let pos = ref(idx);

  let matchesTriggerCharacters = (c) => {
    List.exists((tc) => UChar.eq(c, tc), triggerCharacters);
  };

  let lastCharacter = ref(None);
  let found = ref(false);

  let candidateBase = ref([]);

  while (pos^ >= 0 && !(found^)) {

    let c = Zed_utf8.get(line, pos^);
    lastCharacter := Some(c);

    if (matchesTriggerCharacters(c) || Zed_utf8.is_space(c)) {
      found := true
      incr(pos);
    } else {
      candidateBase := [Zed_utf8.singleton(c), ...candidateBase^];
      decr(pos);
    }
  };

  let base = candidateBase^
  |> String.concat("");

  let baseLength = Zed_utf8.length(base);

  switch (pos^) {
  | -1 => if (baseLength == Index.toInt0(cursor) && baseLength > 0) {
    Some({ index: 0, base })
  } else {
    None
  }
  | v => 
    Some({ index: v, base })
  }
};

