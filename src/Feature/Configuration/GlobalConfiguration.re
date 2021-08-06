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

  let titleBarStyle: decoder([ | `Native | `Custom]) =
    string
    |> map(String.lowercase_ascii)
    |> map(
         fun
         | "native" => `Native
         | "custom" => `Custom
         | _ => `Custom,
       );
};

module Encoders = {
  open Json.Encode;

  let snippetSuggestions: encoder([ | `None | `Inline]) =
    suggestion =>
      switch (suggestion) {
      | `None => string("none")
      | `Inline => string("inline")
      };

  let titleBarStyle: encoder([ | `Native | `Custom]) =
    fun
    | `Native => string("native")
    | `Custom => string("custom");
};

module Codecs = {
  let autoReveal =
    custom(
      ~decode=
        Json.Decode.(
          one_of([
            (
              "autoReveal.bool",
              bool
              |> map(
                   fun
                   | true => `HighlightAndScroll
                   | false => `NoReveal,
                 ),
            ),
            (
              "autoReveal.string",
              string
              |> map(
                   fun
                   | "focusNoScroll" => `HighlightOnly
                   | _ => `NoReveal,
                 ),
            ),
          ])
        ),
      ~encode=
        Json.Encode.(
          fun
          | `HighlightAndScroll => bool(true)
          | `NoReveal => bool(false)
          | `HighlightOnly => string("focusNoScroll")
        ),
    );
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

  let fontSize = {
    let clampSize = size => {
      size < Constants.minimumFontSize ? Constants.minimumFontSize : size;
    };
    custom(
      ~decode=
        Json.Decode.(
          one_of([
            ("fontSize.float", float |> map(clampSize)),
            (
              "fontSize.string",
              string
              |> and_then(str => {
                   str
                   |> float_of_string_opt
                   |> Option.map(clampSize)
                   |> Option.map(succeed)
                   |> Option.value(
                        ~default=fail("Unable to parse font size"),
                      )
                 }),
            ),
          ])
        ),
      ~encode=Json.Encode.float,
    );
  };

  let fontLigatures =
    custom(~decode=FontLigatures.decode, ~encode=FontLigatures.encode);
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

  let lineSpace =
    vim("linespace", lineSpaceSetting => {
      lineSpaceSetting
      |> VimSetting.decode_value_opt(int)
      |> Option.map(LineHeight.padding)
      |> Option.value(~default=LineHeight.default)
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

  let lineHeight =
    setting(
      ~vim=VimSettings.lineSpace,
      "editor.lineHeight",
      custom(~decode=LineHeight.decode, ~encode=LineHeight.encode),
      ~default=LineHeight.default,
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

  let largeFileOptimizations =
    setting("editor.largeFileOptimizations", bool, ~default=true);
};

let vsync =
  setting(
    "vsync",
    custom(
      ~decode=
        Json.Decode.(
          bool
          |> map(
               fun
               | true => Revery.Vsync.Synchronized
               | false => Revery.Vsync.Immediate,
             )
        ),
      ~encode=
        Json.Encode.(
          fun
          | Revery.Vsync.Synchronized => bool(true)
          | Revery.Vsync.Immediate => bool(false)
        ),
    ),
    ~default=Revery.Vsync.Immediate,
  );

module Explorer = {
  let autoReveal =
    setting(
      "explorer.autoReveal",
      Codecs.autoReveal,
      ~default=`HighlightAndScroll,
    );
};

module Files = {
  let exclude =
    setting(
      "files.exclude",
      list(string),
      ~default=["_esy", ".git", "node_modules"],
    );
};

module Search = {
  let followSymlinks = setting("search.followSymlinks", bool, ~default=true);

  let useIgnoreFiles = setting("search.useIgnoreFiles", bool, ~default=true);
};

module Window = {
  // On Windows, default the titlebar style to native, due to various bugs around the custom-rendered window, like:
  // #3730 - Keyboard shortcuts for window movement broken
  // #3071 - Window has no shadow on Windows
  // #3063 - Part of onivim fullscreen is visible on second monitor
  let defaultTitleBarStyle =
    switch (Revery.Environment.os) {
    | Windows(_) => `Native
    | _ => `Custom
    };
  let titleBarStyle =
    setting(
      "window.titleBarStyle",
      custom(~decode=Decoders.titleBarStyle, ~encode=Encoders.titleBarStyle),
      ~default=defaultTitleBarStyle,
    );
};

module Workbench = {
  let activityBarVisible =
    setting("workbench.activityBar.visible", bool, ~default=true);

  let editorShowTabs =
    setting("workbench.editor.showTabs", bool, ~default=true);

  let editorEnablePreview =
    setting("workbench.editor.enablePreview", bool, ~default=true);

  let treeIndent = setting("workbench.tree.indent", int, ~default=5);

  let treeRenderIndentGuides =
    setting("workbench.tree.renderIndentGuides", bool, ~default=true);
};

let contributions = [
  inactiveWindowOpacity.spec,
  animation.spec,
  shadows.spec,
  vsync.spec,
  Editor.codeLensEnabled.spec,
  Editor.largeFileOptimizations.spec,
  Editor.lineHeight.spec,
  Editor.snippetSuggestions.spec,
  Files.exclude.spec,
  Explorer.autoReveal.spec,
  Search.followSymlinks.spec,
  Search.useIgnoreFiles.spec,
  Window.titleBarStyle.spec,
  Workbench.activityBarVisible.spec,
  Workbench.editorShowTabs.spec,
  Workbench.editorEnablePreview.spec,
  Workbench.treeIndent.spec,
  Workbench.treeRenderIndentGuides.spec,
];
