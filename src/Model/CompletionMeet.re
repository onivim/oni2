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
  // Base is the prefix string
  base: string,
  // Meet is the position where we request completions
  meet: Position.t,
};

type t = option(completionMeet);

let show = (v: t) =>
  switch (v) {
  | None => "(None)"
  | Some(m) =>
    Printf.sprintf("Base: |%s| Meet: %s", m.base, m.meet |> Position.show)
  };

let defaultTriggerCharacters = [UChar.of_char('.')];

let createFromLine =
    (
      ~triggerCharacters=defaultTriggerCharacters,
      ~lineNumber=0,
      ~cursor: Index.t,
      line: string,
    ) => {
  let cursorIdx = Index.toInt0(cursor);
  let idx = Stdlib.min(Zed_utf8.length(line) - 1, cursorIdx);
  let pos = ref(idx);

  let matchesTriggerCharacters = c => {
    List.exists(tc => UChar.eq(c, tc), triggerCharacters);
  };

  let lastCharacter = ref(None);
  let found = ref(false);

  let candidateBase = ref([]);

  while (pos^ >= 0 && ! found^) {
    let c = Zed_utf8.get(line, pos^);
    lastCharacter := Some(c);

    if (matchesTriggerCharacters(c)
        || Zed_utf8.is_space(c)
        && List.length(candidateBase^) > 0) {
      found := true;
      incr(pos);
    } else {
      candidateBase := [Zed_utf8.singleton(c), ...candidateBase^];
      decr(pos);
    };
  };

  let base = candidateBase^ |> String.concat("");

  let baseLength = Zed_utf8.length(base);

  switch (pos^) {
  | (-1) =>
    if (baseLength == Index.toInt0(cursor) && baseLength > 0) {
      Some({meet: Position.ofInt0(lineNumber, 0), base});
    } else {
      None;
    }
  | v => Some({meet: Position.ofInt0(lineNumber, v), base})
  };
};

let createFromBufferCursor =
    (
      ~triggerCharacters=defaultTriggerCharacters,
      ~cursor: Position.t,
      buffer: Buffer.t,
    ) => {
  let bufferLines = Buffer.getNumberOfLines(buffer);
  let line0 = Index.toInt0(cursor.line);

  if (line0 < bufferLines) {
    let line = Buffer.getLine(buffer, line0);
    createFromLine(
      ~lineNumber=line0,
      ~triggerCharacters,
      ~cursor=cursor.character,
      line,
    );
  } else {
    None;
  };
};
