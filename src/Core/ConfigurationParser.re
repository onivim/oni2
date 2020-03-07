/*
 * ConfigurationParser.re
 *
 * Resilient parsing for Configuration
 */
open Kernel;
open ConfigurationValues;
open LineNumber;
open Utility;

let parseBool = json =>
  switch (json) {
  | `Bool(v) => v
  | _ => false
  };

let parseInt = (~default=0, json) =>
  switch (json) {
  | `Int(v) => v
  | `Float(v) => int_of_float(v +. 0.5)
  | `String(str) =>
    switch (int_of_string_opt(str)) {
    | None => default
    | Some(v) => v
    }
  | _ => default
  };

let parseFloat = (~default=0., json) =>
  switch (json) {
  | `Int(v) => float_of_int(v)
  | `Float(v) => v
  | `String(str) =>
    let floatMaybe = float_of_string_opt(str);
    let floatFromIntMaybe =
      int_of_string_opt(str) |> Option.map(float_of_int);

    floatMaybe |> OptionEx.or_(floatFromIntMaybe) |> Option.value(~default);
  | _ => default
  };

let parseStringList = json => {
  switch (json) {
  | `List(items) =>
    List.filter_map(
      fun
      | `String(v) => Some(v)
      | _ => None,
      items,
    )
  | `String(v) => [v]
  | _ => []
  };
};

let parseIntList = json => {
  switch (json) {
  | `List(items) =>
    List.fold_left(
      (accum, item) =>
        switch (item) {
        | `Int(v) => [v, ...accum]
        | _ => accum
        },
      [],
      items,
    )
  | _ => []
  };
};

let parseLineNumberSetting = json =>
  switch (json) {
  | `String(v) =>
    switch (v) {
    | "on" => On
    | "off" => Off
    | "relative" => Relative
    | _ => On
    }
  | _ => On
  };

let parseVimUseSystemClipboardSetting = json => {
  let parseItems = items =>
    List.fold_left(
      (accum, item) =>
        switch (item) {
        | `String(v) =>
          switch (String.lowercase_ascii(v)) {
          | "yank" => {...accum, yank: true}
          | "paste" => {...accum, paste: true}
          | "delete" => {...accum, delete: true}
          | _ => accum
          }
        | _ => accum
        },
      {yank: false, delete: false, paste: false},
      items,
    );
  switch (json) {
  | `Bool(true) => {yank: true, delete: true, paste: true}
  | `Bool(false) => {yank: false, delete: false, paste: false}
  | `String(v) => parseItems([`String(v)])
  | `List(items) => parseItems(items)
  | _ => {yank: true, delete: false, paste: false}
  };
};

let parseRenderWhitespace = json =>
  switch (json) {
  | `String(v) =>
    switch (v) {
    | "all" => All
    | "boundary" => Boundary
    | "none" => None
    | _ => All
    }
  | _ => All
  };

let parseEditorFontSize = (~default=Constants.defaultFontSize, json) =>
  json
  |> parseFloat(~default)
  |> (
    result =>
      result > Constants.minimumFontSize ? result : Constants.minimumFontSize
  );

let parseFontSmoothing: Yojson.Safe.t => ConfigurationValues.fontSmoothing =
  json =>
    switch (json) {
    | `String(smoothing) =>
      let smoothing = String.lowercase_ascii(smoothing);
      switch (smoothing) {
      | "none" => None
      | "antialiased" => Antialiased
      | "subpixel-antialiased" => SubpixelAntialiased
      | _ => Default
      };
    | _ => Default
    };

let parseString = (~default="", json) =>
  switch (json) {
  | `String(v) => v
  | _ => default
  };

type parseFunction =
  (ConfigurationValues.t, Yojson.Safe.t) => ConfigurationValues.t;

type configurationTuple = (string, parseFunction);

let configurationParsers: list(configurationTuple) = [
  (
    "editor.fontFamily",
    (config, json) => {
      ...config,
      editorFontFamily:
        parseString(~default=Constants.defaultFontFamily, json),
    },
  ),
  (
    "editor.fontSize",
    (config, json) => {
      ...config,
      editorFontSize: parseEditorFontSize(json),
    },
  ),
  (
    "editor.fontSmoothing",
    (config, json) => {
      ...config,
      editorFontSmoothing: parseFontSmoothing(json),
    },
  ),
  (
    "editor.hover.delay",
    (config, json) => {...config, editorHoverDelay: parseInt(json)},
  ),
  (
    "editor.hover.enabled",
    (config, json) => {...config, editorHoverEnabled: parseBool(json)},
  ),
  (
    "editor.lineNumbers",
    (config, json) => {
      ...config,
      editorLineNumbers: parseLineNumberSetting(json),
    },
  ),
  (
    "editor.matchBrackets",
    (config, json) => {...config, editorMatchBrackets: parseBool(json)},
  ),
  (
    "editor.acceptSuggestionOnEnter",
    (config, json) => {
      ...config,
      editorAcceptSuggestionOnEnter:
        switch (json) {
        | `String("on") => `on
        | `String("off") => `off
        | `String("smart") => `smart
        | _ => `on
        },
    },
  ),
  (
    "editor.minimap.enabled",
    (config, json) => {...config, editorMinimapEnabled: parseBool(json)},
  ),
  (
    "editor.minimap.showSlider",
    (config, json) => {...config, editorMinimapShowSlider: parseBool(json)},
  ),
  (
    "editor.minimap.maxColumn",
    (config, json) => {...config, editorMinimapMaxColumn: parseInt(json)},
  ),
  (
    "editor.minimap.showSlider",
    (config, json) => {...config, editorMinimapShowSlider: parseBool(json)},
  ),
  (
    "editor.detectIndentation",
    (config, json) => {...config, editorDetectIndentation: parseBool(json)},
  ),
  (
    "editor.insertSpaces",
    (config, json) => {...config, editorInsertSpaces: parseBool(json)},
  ),
  (
    "editor.indentSize",
    (config, json) => {...config, editorIndentSize: parseInt(json)},
  ),
  (
    "editor.largeFileOptimizations",
    (config, json) => {
      ...config,
      editorLargeFileOptimizations: parseBool(json),
    },
  ),
  (
    "editor.tabSize",
    (config, json) => {...config, editorTabSize: parseInt(json)},
  ),
  (
    "editor.highlightActiveIndentGuide",
    (config, json) => {
      ...config,
      editorHighlightActiveIndentGuide: parseBool(json),
    },
  ),
  (
    "editor.renderIndentGuides",
    (config, json) => {
      ...config,
      editorRenderIndentGuides: parseBool(json),
    },
  ),
  (
    "editor.renderWhitespace",
    (config, json) => {
      ...config,
      editorRenderWhitespace: parseRenderWhitespace(json),
    },
  ),
  (
    "editor.rulers",
    (config, json) => {...config, editorRulers: parseIntList(json)},
  ),
  (
    "files.exclude",
    (config, json) => {...config, filesExclude: parseStringList(json)},
  ),
  (
    "window.title",
    (config, json) => {...config, windowTitle: parseString(json)},
  ),
  (
    "terminal.integrated.fontFamily",
    (config, json) => {
      ...config,
      terminalIntegratedFontFamily:
        parseString(~default=Constants.defaultFontFamily, json),
    },
  ),
  (
    "terminal.integrated.fontSize",
    (config, json) => {
      ...config,
      terminalIntegratedFontSize:
        parseEditorFontSize(~default=Constants.defaultTerminalFontSize, json),
    },
  ),
  (
    "terminal.integrated.fontSmoothing",
    (config, json) => {
      ...config,
      terminalIntegratedFontSmoothing: parseFontSmoothing(json),
    },
  ),
  (
    "workbench.activityBar.visible",
    (config, json) => {
      ...config,
      workbenchActivityBarVisible: parseBool(json),
    },
  ),
  (
    "workbench.colorTheme",
    (config, json) => {...config, workbenchColorTheme: parseString(json)},
  ),
  (
    "workbench.iconTheme",
    (config, json) => {...config, workbenchIconTheme: parseString(json)},
  ),
  (
    "workbench.editor.showTabs",
    (config, json) => {...config, workbenchEditorShowTabs: parseBool(json)},
  ),
  (
    "workbench.sideBar.visible",
    (config, json) => {...config, workbenchSideBarVisible: parseBool(json)},
  ),
  (
    "workbench.statusBar.visible",
    (config, json) => {
      ...config,
      workbenchStatusBarVisible: parseBool(json),
    },
  ),
  (
    "editor.zenMode.hideTabs",
    (config, json) => {...config, zenModeHideTabs: parseBool(json)},
  ),
  (
    "workbench.tree.indent",
    (config, json) => {...config, workbenchTreeIndent: parseInt(json)},
  ),
  (
    "editor.zenMode.singleFile",
    (config, json) => {...config, zenModeSingleFile: parseBool(json)},
  ),
  (
    "syntax.eagerMaxLines",
    (config, json) => {
      ...config,
      syntaxEagerMaxLines:
        parseInt(~default=Constants.syntaxEagerMaxLines, json),
    },
  ),
  (
    "syntax.eagerMaxLineLength",
    (config, json) => {
      ...config,
      syntaxEagerMaxLineLength:
        parseInt(~default=Constants.syntaxEagerMaxLineLength, json),
    },
  ),
  (
    "ui.shadows",
    (config, json) => {...config, uiShadows: parseBool(json)},
  ),
  ("ui.zoom", (config, json) => {...config, uiZoom: parseFloat(json)}),
  (
    "vim.useSystemClipboard",
    (config, json) => {
      ...config,
      vimUseSystemClipboard: parseVimUseSystemClipboardSetting(json),
    },
  ),
  (
    "vsync",
    (config, json) => {
      ...config,
      vsync:
        parseBool(json) ? Revery.Vsync.Synchronized : Revery.Vsync.Immediate,
    },
  ),
  (
    "experimental.treeSitter",
    (config, json) => {...config, experimentalTreeSitter: parseBool(json)},
  ),
  (
    "experimental.autoClosingPairs",
    (config, json) => {
      ...config,
      experimentalAutoClosingPairs: parseBool(json),
    },
  ),
  (
    "experimental.viml",
    (config, json) => {...config, experimentalVimL: parseStringList(json)},
  ),
  (
    "experimental.editor.smoothScroll",
    (s, v) => {...s, experimentalEditorSmoothScroll: parseBool(v)},
  ),
];

let keyToParser: Hashtbl.t(string, parseFunction) =
  List.fold_left(
    (prev, cur) => {
      let (key, parser) = cur;
      Hashtbl.add(prev, key, parser);
      prev;
    },
    Hashtbl.create(100),
    configurationParsers,
  );

type parseResult = {
  nestedConfigurations: list((string, Yojson.Safe.t)),
  configurationValues: ConfigurationValues.t,
};

let isFiletype = (str: string) => {
  let str = String.trim(str);

  let len = String.length(str);

  if (len >= 3) {
    str.[0] == '[' && str.[len - 1] == ']';
  } else {
    false;
  };
};

let getFiletype = (str: string) => {
  let str = String.trim(str);
  let len = String.length(str);

  String.sub(str, 1, len - 2);
};

let parse: list((string, Yojson.Safe.t)) => parseResult =
  items => {
    List.fold_left(
      (prev, cur) => {
        let (key, json) = cur;
        let {nestedConfigurations, configurationValues} = prev;

        isFiletype(key)
          ? {
            let nestedConfigurations = [
              (getFiletype(key), json),
              ...nestedConfigurations,
            ];
            {nestedConfigurations, configurationValues};
          }
          : (
            switch (Hashtbl.find_opt(keyToParser, key)) {
            | Some(v) => {
                nestedConfigurations,
                configurationValues: v(configurationValues, json),
              }
            | None => prev
            }
          );
      },
      {
        nestedConfigurations: [],
        configurationValues: ConfigurationValues.default,
      },
      items,
    );
  };

let parseNested = (json: Yojson.Safe.t, default: ConfigurationValues.t) => {
  switch (json) {
  | `Assoc(items) =>
    List.fold_left(
      (prev, cur) => {
        let (key, json) = cur;

        isFiletype(key)
          ? prev
          : (
            switch (Hashtbl.find_opt(keyToParser, key)) {
            | Some(v) => v(prev, json)
            | None => prev
            }
          );
      },
      default,
      items,
    )
  | _ => default
  };
};

let ofJson = json => {
  switch (json) {
  | `Assoc(items) =>
    let {configurationValues, nestedConfigurations} = parse(items);

    let perFiletype =
      List.fold_left(
        (prev, cur) => {
          let (key, json) = cur;
          StringMap.add(key, parseNested(json, configurationValues), prev);
        },
        StringMap.empty,
        nestedConfigurations,
      );

    let configuration =
      Configuration.{default: configurationValues, perFiletype};
    Ok(configuration);
  | _ => Error("Incorrect JSON format for configuration")
  };
};

let ofString = str => {
  switch (str |> Yojson.Safe.from_string |> ofJson) {
  | v => v
  | exception (Yojson.Json_error(msg)) => Error(msg)
  };
};

let ofFile = filePath =>
  switch (Yojson.Safe.from_file(filePath) |> ofJson) {
  | v => v
  | exception (Yojson.Json_error(msg)) => Error(msg)
  };
