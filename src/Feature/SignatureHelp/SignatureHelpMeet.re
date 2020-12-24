/**
 * SignatureHelpMeet.re
 *
 * Helper module for finding a 'signature-help' meet - a position matching a trigger or re-trigger character
 */
open EditorCoreTypes;
open Oni_Core;

module CompletionMeet = Feature_LanguageSupport.CompletionMeet;

[@deriving show]
type t = {
  bufferId: int,
  // Location where the signature help meet is
  location: CharacterPosition.t,
  triggerCharacter: [@opaque] Uchar.t,
  isRetrigger: bool,
};

module Internal = {
  let fromLine =
      (
        ~getUchar: int => option(Uchar.t),
        ~index: int,
        ~languageConfiguration,
        ~triggerCharacters,
        ~retriggerCharacters,
      ) => {
    let rec loop = currentIdx =>
      if (currentIdx < 0) {
        None;
      } else {
        let maybeUchar = getUchar(currentIdx);
        switch (maybeUchar) {
        | None => None
        | Some(uchar) =>
          let isTrigger =
            triggerCharacters |> List.exists(trigger => trigger == uchar);
          let isRetrigger =
            retriggerCharacters |> List.exists(retrigger => retrigger == uchar);

          if (isTrigger) {
            Some((currentIdx, uchar, false));
          } else if (isRetrigger) {
            Some((currentIdx, uchar, true));
          } else if (LanguageConfiguration.isWordCharacter(
                       uchar,
                       languageConfiguration,
                     )
                     || Uucp.White.is_white_space(uchar)) {
            // If a valid character or whitespace, keep looking for a trigger
            loop(
              currentIdx - 1,
            );
          } else {
            None;
                // Hit a character that is not a trigger character of a valid word character... no meet found.
          };
        };
      };

    loop(index);
  };

  let fromString =
      (
        ~index,
        ~languageConfiguration,
        ~triggerCharacters,
        ~retriggerCharacters,
        str,
      ) => {
    let getUchar = idx =>
      if (idx < 0) {
        None;
      } else {
        Some(Uchar.of_char(str.[idx]));
      };

    fromLine(
      ~getUchar,
      ~index,
      ~languageConfiguration,
      ~retriggerCharacters,
      ~triggerCharacters,
    );
  };

  let%test_module "fromString" =
    (module
     {
       let getMeet =
         fromString(
           ~languageConfiguration=LanguageConfiguration.default,
           ~triggerCharacters=[Uchar.of_char('(')],
           ~retriggerCharacters=[Uchar.of_char(',')],
         );
       let%test "simple no-meet test" = {
         getMeet(~index=2, "abc") == None;
       };

       let%test "trigger meet, no extra characters" = {
         getMeet(~index=3, "abc(") == Some((3, Uchar.of_char('('), false));
       };
       let%test "trigger meet, extra word characters and space" = {
         getMeet(~index=6, "abc( ab")
         == Some((3, Uchar.of_char('('), false));
       };
       let%test "retrigger meet, no extra characters" = {
         getMeet(~index=5, "abc(a,") == Some((5, Uchar.of_char(','), true));
       };
       let%test "retrigger meet, extra characters" = {
         getMeet(~index=7, "abc(a, b")
         == Some((5, Uchar.of_char(','), true));
       };
       let%test "no meet, after closing character" = {
         getMeet(~index=8, "abc(a, b)") == None;
       };
     });
};

let fromBufferPosition =
    (
      ~languageConfiguration,
      ~triggerCharacters,
      ~retriggerCharacters,
      ~position: CharacterPosition.t,
      buffer: Buffer.t,
    ) => {
  let bufferLines = Buffer.getNumberOfLines(buffer);
  let line0 = EditorCoreTypes.LineNumber.toZeroBased(position.line);

  if (line0 < bufferLines) {
    let line = Buffer.getLine(line0, buffer);
    let getUchar = idx =>
      BufferLine.getUchar(~index=CharacterIndex.ofInt(idx), line);
    Internal.fromLine(
      ~getUchar,
      ~index=CharacterIndex.toInt(position.character) - 1,
      ~triggerCharacters,
      ~retriggerCharacters,
      ~languageConfiguration,
    )
    |> Option.map(((idx, triggerCharacter, isRetrigger)) =>
         {
           bufferId: Oni_Core.Buffer.getId(buffer),
           location:
             EditorCoreTypes.CharacterPosition.{
               line: position.line,
               character: CharacterIndex.ofInt(idx + 1),
             },
           triggerCharacter,
           isRetrigger,
         }
       );
  } else {
    None;
  };
};
