/**
 * SignatureHelpMeet.re
 *
 * Helper module for finding a 'signature-help' meet - a position matching a trigger or re-trigger character
 */
open EditorCoreTypes;
open Oni_Core;

module CompletionMeet = Feature_LanguageSupport.CompletionMeet;

type t = {
  bufferId: int,
  // Location where the signature help meet is
  location: CharacterPosition.t,
  isRetrigger: bool,
};

module Internal = {

  let fromLine =
  (
    ~getUchar: int => option(Uchar.t),
    ~index: int,
    ~languageConfiguration,
    ~triggerCharacters,
    ~retriggerCharacters
  ) => {
    None
  }

  let fromString =
    (
      ~index,
      ~languageConfiguration,
      ~triggerCharacters,
      ~retriggerCharacters,
      str,
    ) => {
      let getUchar = idx => if(idx < 0) {
        None
      } else {
        Some(Uchar.of_char(str[idx]))
      };

      fromLine(
        ~getUchar,
        ~index,
        ~languageConfiguration,
        ~retriggerCharacters
      );
    };

    let%test_module "fromString" = (module {
      let getMeet = fromString(
        ~languageConfiguration=LanguageConfiguration.initial,
        ~triggerCharacters=[Uchar.of_char('(')],
        ~retriggerCharacters=[Uchar.of_char(',')]
      );
      let%test "basic test" = {
        getMeet(~index=2, "abc") == None
      };
    });
}

let fromBufferPosition =
    (
      ~languageConfiguration,
      ~triggerCharacters,
      ~retriggerCharacters,
      ~position: CharacterPosition.t,
      buffer: Buffer.t,
    ) => {
  let maybeTriggerMeet =
    CompletionMeet.fromBufferPosition(
      ~languageConfiguration,
      ~triggerCharacters,
      ~position,
      buffer,
    );
  let maybeRetriggerMeet =
    CompletionMeet.fromBufferPosition(
      ~languageConfiguration,
      ~triggerCharacters=retriggerCharacters,
      ~position,
      buffer,
    );

  switch (maybeTriggerMeet, maybeRetriggerMeet) {
  | (None, None) => None
  | (Some({bufferId, location, _}), None) =>
    Some({bufferId, location, isRetrigger: false})
  | (None, Some({bufferId, location, _})) =>
    Some({bufferId, location, isRetrigger: true})
  | (
      Some({bufferId, location: triggerLocation, _}),
      Some({location: retriggerLocation, _}),
    ) =>
    CharacterPosition.(
      if (EditorCoreTypes.(
            CharacterIndex.(
              retriggerLocation.character > triggerLocation.character
            )
          )) {
        Some({bufferId, location: retriggerLocation, isRetrigger: true});
      } else {
        Some({bufferId, location: triggerLocation, isRetrigger: false});
      }
    )
  };
};
