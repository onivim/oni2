/*
 * ConfigurationParser.re
 *
 * Resilient parsing for Configuration
 */
open Kernel;
open ConfigurationValues;
open Utility;

let parseBool = json =>
  switch (json) {
  | `Bool(v) => v
  | _ => false
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

let parseAutoClosingBrackets:
  Yojson.Safe.t => ConfigurationValues.autoClosingBrackets =
  json =>
    switch (json) {
    | `Bool(true) => LanguageDefined
    | `Bool(false) => Never
    | `String(autoClosingBrackets) =>
      let autoClosingBrackets = String.lowercase_ascii(autoClosingBrackets);
      switch (autoClosingBrackets) {
      | "never" => Never
      | "languagedefined" => LanguageDefined
      | _ => Never
      };
    | _ => Never
    };

let parseString = (~default="", json) =>
  switch (json) {
  | `String(v) => v
  | _ => default
  };

let parseFontLigatures = json =>
  switch (json) {
  | `Bool(_) as bool => bool
  | `String(str) =>
    open Angstrom;

    let quoted = p => char('\'') *> p <* char('\'');

    let isAlphaNumeric = (
      fun
      | 'a'..'z'
      | 'A'..'Z'
      | '0'..'9' => true
      | _ => false
    );

    let alphaString = take_while1(isAlphaNumeric);

    let feature = quoted(alphaString);
    let spaces = many(char(' '));

    let parse = sep_by(char(',') <* spaces, feature);

    switch (Angstrom.parse_string(~consume=All, parse, str)) {
    | Ok(list) => `List(list)
    | Error(_) => `Bool(true)
    };
  | _ => `Bool(true)
  };

let parseAutoReveal = json =>
  switch (json) {
  | `Bool(true) => `HighlightAndScroll
  | `Bool(false) => `NoReveal
  | `String("focusNoScroll") => `HighlightOnly
  | _ => `NoReveal
  };

type parseFunction =
  (ConfigurationValues.t, Yojson.Safe.t) => ConfigurationValues.t;

type configurationTuple = (string, parseFunction);

let configurationParsers: list(configurationTuple) = [
  (
    "editor.autoClosingBrackets",
    (config, json) => {
      ...config,
      editorAutoClosingBrackets: parseAutoClosingBrackets(json),
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
    "editor.fontLigatures",
    (config, json) => {
      ...config,
      editorFontLigatures: parseFontLigatures(json),
    },
  ),
  (
    "editor.largeFileOptimizations",
    (config, json) => {
      ...config,
      editorLargeFileOptimizations: parseBool(json),
    },
  ),
  (
    "explorer.autoReveal",
    (config, json) => {
      ...config,
      explorerAutoReveal: parseAutoReveal(json),
    },
  ),
  (
    "files.exclude",
    (config, json) => {...config, filesExclude: parseStringList(json)},
  ),
  (
    "terminal.integrated.fontFamily",
    (config, json) => {
      ...config,
      terminalIntegratedFontFile:
        parseString(~default=Constants.defaultFontFile, json),
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
    "workbench.editor.enablePreview",
    (config, json) => {
      ...config,
      workbenchEditorEnablePreview: parseBool(json),
    },
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
    "editor.zenMode.singleFile",
    (config, json) => {...config, zenModeSingleFile: parseBool(json)},
  ),
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
    "experimental.viml",
    (config, json) => {...config, experimentalVimL: parseStringList(json)},
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
