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
