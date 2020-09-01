open Revery;
open Oni_Core;
open Oni_Core.Utility;
open Config.Schema;

module CustomDecoders: {
  let whitespace:
    Config.Schema.codec([ | `All | `Boundary | `Selection | `None]);
  let lineNumbers:
    Config.Schema.codec([ | `On | `Relative | `RelativeOnly | `Off]);
  let time: Config.Schema.codec(Time.t);
  let fontSize: Config.Schema.codec(float);
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
               | "selection" => `Selection
               | "all"
               | _ => `Selection,
             )
        ),
      ~encode=
        Json.Encode.(
          fun
          | `None => string("none")
          | `Boundary => string("boundary")
          | `Selection => string("selection")
          | `All => string("all")
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

  let lineNumbers =
    custom(
      ~decode=
        Json.Decode.(
          string
          |> map(
               fun
               | "off" => `Off
               | "relative" => `Relative
               | "relative-only" => `RelativeOnly
               | "on"
               | _ => `On,
             )
        ),
      ~encode=
        Json.Encode.(
          fun
          | `Off => string("off")
          | `Relative => string("relative")
          | `RelativeOnly => string("relative-only")
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

module VimSettings = {
  open VimSetting.Schema;
  let smoothScroll =
    vim("smoothscroll", scrollSetting => {
      scrollSetting
      |> VimSetting.decode_value_opt(bool)
      |> Option.value(~default=false)
    });

  let minimap =
    vim("minimap", scrollSetting => {
      scrollSetting
      |> VimSetting.decode_value_opt(bool)
      |> Option.value(~default=false)
    });

  let guifont =
    vim("guifont", guifontSetting => {
      guifontSetting
      |> VimSetting.decode_value_opt(font)
      |> Option.map(({fontFamily, _}: VimSetting.fontDescription) =>
           fontFamily
         )
      |> Option.value(~default="JetBrainsMono-Regular.ttf")
    });

  let lineSpace =
    vim("linespace", lineSpaceSetting => {
      lineSpaceSetting
      |> VimSetting.decode_value_opt(int)
      |> Option.map(LineHeight.padding)
      |> Option.value(~default=LineHeight.default)
    });

  let lineNumbers =
    vim2("relativenumber", "number", (maybeRelative, maybeNumber) => {
      let maybeRelativeBool =
        maybeRelative |> OptionEx.flatMap(VimSetting.decode_value_opt(bool));
      let maybeNumberBool =
        maybeNumber |> OptionEx.flatMap(VimSetting.decode_value_opt(bool));

      let justRelative =
        fun
        | Some(true) => Some(`Relative)
        | Some(false) => Some(`Off)
        | None => None;

      let justAbsolute =
        fun
        | Some(true) => Some(`On)
        | Some(false) => Some(`Off)
        | None => None;

      OptionEx.map2(
        (maybeRelativeBool, maybeVimSettingBool) => {
          switch (maybeRelativeBool, maybeVimSettingBool) {
          | (true, true) => `Relative
          | (true, false) => `RelativeOnly
          | (false, true) => `On
          | (false, false) => `Off
          }
        },
        maybeRelativeBool,
        maybeNumberBool,
      )
      |> OptionEx.or_lazy(() => maybeRelativeBool |> justRelative)
      |> OptionEx.or_lazy(() => maybeNumberBool |> justAbsolute);
    });
};

open CustomDecoders;

let detectIndentation =
  setting("editor.detectIndentation", bool, ~default=true);
let fontFamily =
  setting(
    ~vim=VimSettings.guifont,
    "editor.fontFamily",
    string,
    ~default="JetBrainsMono-Regular.ttf",
  );
let fontLigatures = setting("editor.fontLigatures", bool, ~default=true);
let fontSize = setting("editor.fontSize", fontSize, ~default=14.);
let lineHeight =
  setting(
    ~vim=VimSettings.lineSpace,
    "editor.lineHeight",
    custom(~decode=LineHeight.decode, ~encode=LineHeight.encode),
    ~default=LineHeight.default,
  );
let largeFileOptimization =
  setting("editor.largeFileOptimizations", bool, ~default=true);
let highlightActiveIndentGuide =
  setting("editor.highlightActiveIndentGuide", bool, ~default=true);
let indentSize = setting("editor.indentSize", int, ~default=4);
let insertSpaces = setting("editor.insertSpaces", bool, ~default=true);
let lineNumbers =
  setting(
    ~vim=VimSettings.lineNumbers,
    "editor.lineNumbers",
    lineNumbers,
    ~default=`On,
  );
let matchBrackets = setting("editor.matchBrackets", bool, ~default=true);
let renderIndentGuides =
  setting("editor.renderIndentGuides", bool, ~default=true);
let renderWhitespace =
  setting("editor.renderWhitespace", whitespace, ~default=`Selection);
let rulers = setting("editor.rulers", list(int), ~default=[]);
let scrollShadow = setting("editor.scrollShadow", bool, ~default=true);
let smoothScroll =
  setting(
    ~vim=VimSettings.smoothScroll,
    "editor.smoothScroll",
    bool,
    ~default=true,
  );

let tabSize = setting("editor.tabSize", int, ~default=4);
let yankHighlightAnimation =
  setting("editor.yankHighlightAnimation", bool, ~default=true);

module Hover = {
  let enabled = setting("editor.hover.enabled", bool, ~default=true);
  let delay = setting("editor.hover.delay", time, ~default=Time.zero);
};

module Minimap = {
  let enabled =
    setting(
      ~vim=VimSettings.minimap,
      "editor.minimap.enabled",
      bool,
      ~default=true,
    );
  let maxColumn = setting("editor.minimap.maxColumn", int, ~default=120);
  let showSlider = setting("editor.minimap.showSlider", bool, ~default=true);
};

module ZenMode = {
  let hideTabs = setting("editor.zenMode.hideTabs", bool, ~default=true);
  let singleFile = setting("editor.zenMode.singleFile", bool, ~default=true);
};

module Experimental = {
  let cursorSmoothCaretAnimation =
    setting(
      "experimental.editor.cursorSmoothCaretAnimation",
      bool,
      ~default=false,
    );
};

let contributions = [
  detectIndentation.spec,
  fontFamily.spec,
  fontLigatures.spec,
  fontSize.spec,
  lineHeight.spec,
  largeFileOptimization.spec,
  highlightActiveIndentGuide.spec,
  indentSize.spec,
  insertSpaces.spec,
  lineNumbers.spec,
  matchBrackets.spec,
  renderIndentGuides.spec,
  renderWhitespace.spec,
  rulers.spec,
  scrollShadow.spec,
  smoothScroll.spec,
  tabSize.spec,
  yankHighlightAnimation.spec,
  Hover.enabled.spec,
  Hover.delay.spec,
  Minimap.enabled.spec,
  Minimap.maxColumn.spec,
  Minimap.showSlider.spec,
  ZenMode.hideTabs.spec,
  ZenMode.singleFile.spec,
  Experimental.cursorSmoothCaretAnimation.spec,
];
