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
};

module Codecs = {
  let fontWeightDecoder =
    Json.Decode.(
      one_of([
        (
          "fontWeight.int",
          int
          |> map(
               fun
               | 100 => Revery.Font.Weight.Thin
               | 200 => Revery.Font.Weight.UltraLight
               | 300 => Revery.Font.Weight.Light
               | 400 => Revery.Font.Weight.Normal
               | 500 => Revery.Font.Weight.Medium
               | 600 => Revery.Font.Weight.SemiBold
               | 700 => Revery.Font.Weight.Bold
               | 800 => Revery.Font.Weight.UltraBold
               | 900 => Revery.Font.Weight.Heavy
               | _ => Revery.Font.Weight.Normal,
             ),
        ),
        (
          "fontWeight.string",
          string
          |> map(
               fun
               | "100" => Revery.Font.Weight.Thin
               | "200" => Revery.Font.Weight.UltraLight
               | "300" => Revery.Font.Weight.Light
               | "400"
               | "normal" => Revery.Font.Weight.Normal
               | "500" => Revery.Font.Weight.Medium
               | "600" => Revery.Font.Weight.SemiBold
               | "700"
               | "bold" => Revery.Font.Weight.Bold
               | "800" => Revery.Font.Weight.UltraBold
               | "900" => Revery.Font.Weight.Heavy
               | _ => Revery.Font.Weight.Normal,
             ),
        ),
      ])
    );
  let fontWeight =
    custom(
      ~decode=fontWeightDecoder,
      ~encode=
        Json.Encode.(
          t =>
            switch (t) {
            | Revery.Font.Weight.Normal => string("normal")
            | Revery.Font.Weight.Bold => string("bold")
            | _ => string(string_of_int(Revery.Font.Weight.toInt(t)))
            }
        ),
    );

  let fontSize =
    custom(
      ~decode=
        Json.Decode.(
          float
          |> map(size => {
               size < Constants.minimumFontSize
                 ? Constants.minimumFontSize : size
             })
        ),
      ~encode=Json.Encode.float,
    );
}

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
};

module Experimental = {};

let contributions = [
  inactiveWindowOpacity.spec,
  animation.spec,
  shadows.spec,
  Editor.codeLensEnabled.spec,
];
