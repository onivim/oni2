/*
 * CompletionMeet.re
 *
 * Helpers for finding the completion 'meet' (the location where we should request completions),
 * based on the current line and trigger characters
 */

open Oni_Core;

open CamomileBundled.Camomile;
module Zed_utf8 = ZedBundled;

type t = {
  // Base is the prefix string
  base: string,
  // Position to request completions
  position: Position.t,
};

let toString = (meet: t) =>
  Printf.sprintf(
    "Base: |%s| Meet: %s",
    meet.base,
    meet.position |> Position.show,
  );

let create = (~position, ~base) => { position, base };

let defaultTriggerCharacters = [UChar.of_char('.')];

let getPosition = meet => meet.position;
let getBase = meet => meet.base;

let createFromLine =
    (
      ~triggerCharacters=defaultTriggerCharacters,
      ~lineNumber=0,
      ~index: Index.t,
      line: string,
    ) => {
  let cursorIdx = Index.toInt0(index);
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
    if (baseLength == cursorIdx && baseLength > 0) {
      Some({position: Position.ofInt0(lineNumber, 0), base});
    } else {
      None;
    }
  | v => Some({position: Position.ofInt0(lineNumber, v), base})
  };
};

let createFromBufferPosition =
    (
      ~triggerCharacters=defaultTriggerCharacters,
      ~position: Position.t,
      buffer: Buffer.t,
    ) => {
  let bufferLines = Buffer.getNumberOfLines(buffer);
  let line0 = Index.toInt0(position.line);

  if (line0 < bufferLines) {
    let line = Buffer.getLine(buffer, line0);
    createFromLine(
      ~lineNumber=line0,
      ~triggerCharacters,
      ~index=position.character,
      line,
    );
  } else {
    None;
  };
};
