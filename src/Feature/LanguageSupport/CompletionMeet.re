/*
 * CompletionMeet.re
 *
 * Helpers for finding the completion 'meet' (the location where we should request completions),
 * based on the current line and trigger characters
 */

open EditorCoreTypes;
open Oni_Core;

module Zed_utf8 = ZedBundled;
[@deriving show]
type t = {
  bufferId: int,
  // Base is the prefix string
  base: string,
  // Meet is the location where we request completions
  location: CharacterPosition.t,
};

let shiftMeet = (~edits, meet) => {
  edits
  |> List.fold_left(
       (meet, edit: Exthost.Edit.SingleEditOperation.t) => {
         let editStopLine1 = Exthost.OneBasedRange.(edit.range.endLineNumber);
         let meetLine1 =
           EditorCoreTypes.LineNumber.toOneBased(meet.location.line);

         if (editStopLine1 <= meetLine1) {
           let delta = Exthost.Edit.SingleEditOperation.deltaLineCount(edit);
           {
             ...meet,
             location:
               CharacterPosition.{
                 line: EditorCoreTypes.LineNumber.(meet.location.line + delta),
                 character: meet.location.character,
               },
           };
         } else {
           meet;
         };
       },
       meet,
     );
};

let matches = (a, b) => {
  a.bufferId == b.bufferId && a.location == b.location;
};

let toString = (meet: t) =>
  Printf.sprintf(
    "Base: |%s| Meet: %s",
    meet.base,
    meet.location |> CharacterPosition.show,
  );

let defaultTriggerCharacters = [Uchar.of_char('.')];

let fromLine =
    (
      ~languageConfiguration,
      ~triggerCharacters=defaultTriggerCharacters,
      ~lineNumber=0,
      ~bufferId,
      ~index: CharacterIndex.t,
      line: BufferLine.t,
    ) => {
  let cursorIdx = CharacterIndex.toInt(index);
  let idx =
    Stdlib.min(
      BufferLine.lengthSlow(line) - 1,
      // Rewind the cursor back a character,
      // because in insert mode the position is actually 'after'
      // the last character.
      cursorIdx - 1,
    );

  let matchesTriggerCharacters = c => {
    List.exists(tc => Uchar.equal(c, tc), triggerCharacters);
  };

  let rec loop = (acc, currentPos) =>
    if (currentPos < 0) {
      (false, [], 0);
    } else {
      let uchar =
        BufferLine.getUcharExn(
          ~index=CharacterIndex.ofInt(currentPos),
          line,
        );
      let matchesTrigger = matchesTriggerCharacters(uchar);
      let isWordCharacter =
        LanguageConfiguration.isWordCharacter(uchar, languageConfiguration);
      if (matchesTrigger || !isWordCharacter) {
        // If the cursor is after a trigger character, like console.|,
        // an empty string is valid. However, if it's just a non-word character,
        // we require at least a single character for a meet.
        let validLength = matchesTriggerCharacters(uchar) ? 0 : 1;
        (List.length(acc) >= validLength, acc, currentPos + 1);
      } else if (currentPos == 0) {
        let all = [uchar, ...acc];
        (List.length(all) >= 1, all, currentPos);
      } else {
        loop([uchar, ...acc], currentPos - 1);
      };
    };

  let (isValid, characters, pos) = loop([], idx);
  let base = Zed_utf8.implode(characters);

  if (isValid) {
    let meet = {
      bufferId,
      location:
        CharacterPosition.{
          line: EditorCoreTypes.LineNumber.ofZeroBased(lineNumber),
          character: CharacterIndex.ofInt(pos),
        },
      base,
    };
    Some(meet);
  } else {
    None;
  };
};

let fromBufferPosition =
    (
      ~languageConfiguration,
      ~triggerCharacters=defaultTriggerCharacters,
      ~position: CharacterPosition.t,
      buffer: Buffer.t,
    ) => {
  let bufferLines = Buffer.getNumberOfLines(buffer);
  let line0 = EditorCoreTypes.LineNumber.toZeroBased(position.line);

  if (line0 < bufferLines) {
    let line = Buffer.getLine(line0, buffer);
    fromLine(
      ~languageConfiguration,
      ~bufferId=Buffer.getId(buffer),
      ~lineNumber=line0,
      ~triggerCharacters,
      ~index=position.character,
      line,
    );
  } else {
    None;
  };
};
