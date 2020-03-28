open Revery;
open Oni_Core;
open Config.Schema;

module CustomDecoders: {
  let whitespace: Config.Schema.codec([ | `All | `Boundary | `None]);
  let lineNumbers: Config.Schema.codec([ | `On | `Relative | `Off]);
  let time: Config.Schema.codec(Time.t);
} = {
  let whitespace =
    custom(
      ~decode=
        Json.Decode.(
          string
          |> map(
               fun
               | "none" => `None
               | "boundary" => `Boundary
               | "all"
               | _ => `All,
             )
        ),
      ~encode=
        Json.Encode.(
          fun
          | `None => string("none")
          | `Boundary => string("boundary")
          | `All => string("all")
        ),
    );

  let lineNumbers =
    custom(
      ~decode=
        Json.Decode.(
          string
          |> map(
               fun
               | "off" => `Off
               | "relative" => `Relative
               | "on"
               | _ => `On,
             )
        ),
      ~encode=
        Json.Encode.(
          fun
          | `Off => string("off")
          | `Relative => string("relative")
          | `On => string("on")
        ),
    );

  let time =
    custom(
      ~decode=Json.Decode.(int |> map(Time.ms)),
      ~encode=
        Json.Encode.(t => t |> Time.toFloatSeconds |> int_of_float |> int),
    );
};

open CustomDecoders;

let detectIndentation =
  setting("editor.detectIndentation", bool, ~default=true);
let fontFamily =
  setting("editor.fontFamily", string, ~default="FiraCode-Regular.ttf");
let fontSize = setting("editor.fontSize", int, ~default=14);
let largeFileOptimization =
  setting("editor.largeFileOptimizations", bool, ~default=true);
let highlightActiveIndentGuide =
  setting("editor.highlightActiveIndentGuide", bool, ~default=true);
let indentSize = setting("editor.indentSize", int, ~default=4);
let insertSpaces = setting("editor.insertSpaces", bool, ~default=false);
let lineNumbers = setting("editor.lineNumbers", lineNumbers, ~default=`On);
let matchBrackets = setting("editor.matchBrackets", bool, ~default=true);
let renderIndentGuides =
  setting("editor.renderIndentGuides", bool, ~default=true);
let renderWhitespace =
  setting("editor.renderWhitespace", whitespace, ~default=`All);
let rulers = setting("editor.rulers", list(int), ~default=[]);
let tabSize = setting("editor.tabSize", int, ~default=4);

module Hover = {
  let enabled = setting("editor.hover.enabled", bool, ~default=true);
  let delay = setting("editor.hover.delay", time, ~default=Time.zero);
};

module Minimap = {
  let enabled = setting("editor.minimap.enabled", bool, ~default=true);
  let maxColumn = setting("editor.minimap.maxColumn", int, ~default=120);
  let showSlider = setting("editor.minimap.showSlider", bool, ~default=true);
};

module ZenMode = {
  let hideTabs = setting("editor.zenMode.hideTabs", bool, ~default=true);
  let singleFile = setting("editor.zenMode.singleFile", bool, ~default=true);
};

module Experimental = {
  let editorSmoothScroll =
    setting("experimental.editor.smoothScroll", bool, ~default=false);

  let editorSmoothCursor =
    setting("experimental.editor.smoothCursor", bool, ~default=false);
};

let contributions = [
  detectIndentation.spec,
  fontFamily.spec,
  fontSize.spec,
  largeFileOptimization.spec,
  highlightActiveIndentGuide.spec,
  indentSize.spec,
  insertSpaces.spec,
  lineNumbers.spec,
  matchBrackets.spec,
  renderIndentGuides.spec,
  renderWhitespace.spec,
  rulers.spec,
  tabSize.spec,
  Hover.enabled.spec,
  Hover.delay.spec,
  Minimap.enabled.spec,
  Minimap.maxColumn.spec,
  Minimap.showSlider.spec,
  ZenMode.hideTabs.spec,
  ZenMode.singleFile.spec,
];
