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
  
  while (pos^ >= 0 && !(found^)) {

    let c = Zed_utf8.get(line, pos^);
    lastCharacter := Some(c);

    if (matchesTriggerCharacters(c) || Zed_utf8.is_space(c)) {
      found := true
    } else {
      decr(pos);
    }
  };

  switch (pos^) {
  | -1 => None
  | v => Some({ index: v, base: "" })
  }
};

