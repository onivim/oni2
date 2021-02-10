/*
 * GlobalConfiguration.re
 *
 * Configuration that spans multiple features
 */

open Oni_Core;
open Config.Schema;

module Decoders = {
  open Json.Decode;

  let inactiveWindowOpacity =
    one_of([
      ("float", float),
      (
        "bool",
        bool
        |> map(
             fun
             | true => 0.75
             | false => 1.0,
           ),
      ),
    ]);

  let snippetSuggestions: decoder([ | `None | `Inline]) =
    one_of([
      (
        "bool",
        bool
        |> map(
             fun
             | false => `None
             | true => `Inline,
           ),
      ),
      (
        "string",
        string
        |> map(
             fun
             | "none" => `None
             | "inline" => `Inline
             | _ => `None,
           ),
      ),
    ]);
};

module Encoders = {
  open Json.Encode;

  let snippetSuggestions: encoder([ | `None | `Inline]) =
    suggestion =>
      switch (suggestion) {
      | `None => string("none")
      | `Inline => string("inline")
      };
};

module Custom = {
  let inactiveWindowOpacity =
    custom(~decode=Decoders.inactiveWindowOpacity, ~encode=Json.Encode.float);
};

let inactiveWindowOpacity =
  setting(
    "oni.inactiveWindowOpacity",
    Custom.inactiveWindowOpacity,
    ~default=0.75,
  );

let animation = setting("ui.animation", bool, ~default=true);

let shadows = setting("ui.shadows", bool, ~default=true);

module VimSettings = {
  open Config.Schema;
  open VimSetting.Schema;

  let codeLens =
    vim("codelens", codeLensSetting => {
      codeLensSetting
      |> VimSetting.decode_value_opt(bool)
      |> Option.value(~default=false)
    });
};

module Editor = {
  let codeLensEnabled =
    setting(
      ~vim=VimSettings.codeLens,
      "editor.codeLens",
      bool,
      ~default=true,
    );

  let snippetSuggestions =
    setting(
      "editor.snippetSuggestions",
      custom(
        ~encode=Encoders.snippetSuggestions,
        ~decode=Decoders.snippetSuggestions,
      ),
      ~default=`None,
    );
};

let contributions = [
  inactiveWindowOpacity.spec,
  animation.spec,
  shadows.spec,
  Editor.codeLensEnabled.spec,
  Editor.snippetSuggestions.spec,
];
