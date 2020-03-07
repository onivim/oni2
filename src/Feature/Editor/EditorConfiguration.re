open Revery;
open Oni_Core;
open Config.Schema;

module CustomDecoders: {
  let whitespace:
    Config.Schema.decoder(option([ | `All | `Boundary | `None]));
  let lineNumbers:
    Config.Schema.decoder(option([ | `On | `Relative | `Off]));
  let time: Config.Schema.decoder(option(Time.t));
} = {
  open Json.Decode;

  let whitespace =
    custom(
      string
      |> map(
           fun
           | "none" => `None
           | "boundary" => `Boundary
           | "all"
           | _ => `All,
         ),
    );

  let lineNumbers =
    custom(
      string
      |> map(
           fun
           | "off" => `Off
           | "relative" => `Relative
           | "on"
           | _ => `On,
         ),
    );

  let time = custom(int |> map(Time.ms));
};

open CustomDecoders;

let detectIndentation =
  setting("editor.detectIndentation", bool |> default(true));
let fontFamily =
  setting("editor.fontFamily", string |> default("FiraCode-Regular.ttf"));
let fontSize = setting("editor.fontSize", int |> default(14));
let largeFileOptimization =
  setting("editor.largeFileOptimizations", bool |> default(true));
let highlightActiveIndentGuide =
  setting("editor.highlightActiveIndentGuide", bool |> default(true));
let indentSize = setting("editor.indentSize", int |> default(4));
let insertSpaces = setting("editor.insertSpaces", bool |> default(false));
let lineNumbers = setting("editor.lineNumbers", lineNumbers |> default(`On));
let matchBrackets = setting("editor.matchBrackets", bool |> default(true));
let renderIndentGuides =
  setting("editor.renderIndentGuides", bool |> default(true));
let renderWhitespace =
  setting("editor.renderWhitespace", whitespace |> default(`All));
let rulers = setting("editor.rulers", list(int) |> default([]));
let tabSize = setting("editor.tabSize", int |> default(4));

module Hover = {
  let enabled = setting("editor.hover.enabled", bool |> default(true));
  let delay = setting("editor.hover.delay", time |> default(Time.zero));
};

module Minimap = {
  let enabled = setting("editor.minimap.enabled", bool |> default(true));
  let maxColumn = setting("editor.minimap.maxColumn", int |> default(120));
  let showSlider =
    setting("editor.minimap.showSlider", bool |> default(true));
};

module ZenMode = {
  let hideTabs = setting("editor.zenMode.hideTabs", bool |> default(true));
  let singleFile =
    setting("editor.zenMode.singleFile", bool |> default(true));
};
