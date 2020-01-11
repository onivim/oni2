/*
 * CompletionMeet.re
 *
 * Helpers for finding the completion 'meet' (the location where we should request completions),
 * based on the current line and trigger characters
 */

open EditorCoreTypes;
open Oni_Core;

open CamomileBundled.Camomile;
module Zed_utf8 = ZedBundled;

type t = {
  bufferId: int,
  // Base is the prefix string
  base: string,
  // Meet is the location where we request completions
  location: Location.t,
};

let toString = (meet: t) =>
  Printf.sprintf(
    "Base: |%s| Meet: %s",
    meet.base,
    meet.location |> Location.show,
  );

let defaultTriggerCharacters = [UChar.of_char('.')];

let fromLine =
    (
      ~triggerCharacters=defaultTriggerCharacters,
      ~lineNumber=0,
      ~bufferId,
      ~index: Index.t,
      line: string,
    ) => {
  let cursorIdx = Index.toZeroBased(index);
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
      Some({
        bufferId,
        location:
          Location.{
            line: Index.fromZeroBased(lineNumber),
            column: Index.zero,
          },
        base,
      });
    } else {
      None;
    }
  | v =>
    Some({
      bufferId,
      location:
        Location.{
          line: Index.fromZeroBased(lineNumber),
          column: Index.fromZeroBased(v),
        },
      base,
    })
  };
};

let fromBufferLocation =
    (
      ~triggerCharacters=defaultTriggerCharacters,
      ~location: Location.t,
      buffer: Buffer.t,
    ) => {
  let bufferLines = Buffer.getNumberOfLines(buffer);
  let line0 = Index.toZeroBased(location.line);

  if (line0 < bufferLines) {
    let line = Buffer.getLine(buffer, line0);
    fromLine(
      ~bufferId=Buffer.getId(buffer),
      ~lineNumber=line0,
      ~triggerCharacters,
      ~index=location.column,
      line,
    );
  } else {
    None;
  };
};
