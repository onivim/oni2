/*
 * ConfigurationParser.re
 *
 * Resilient parsing for Configuration
 */
open Oni_Core;
open LegacyConfigurationValues;

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

type parseFunction =
  (LegacyConfigurationValues.t, Yojson.Safe.t) => LegacyConfigurationValues.t;

type configurationTuple = (string, parseFunction);

let configurationParsers: list(configurationTuple) = [
  (
    "files.exclude",
    (config, json) => {...config, filesExclude: parseStringList(json)},
  ),
  (
    "vim.useSystemClipboard",
    (config, json) => {
      ...config,
      vimUseSystemClipboard: parseVimUseSystemClipboardSetting(json),
    },
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
  configurationValues: LegacyConfigurationValues.t,
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
        configurationValues: LegacyConfigurationValues.default,
      },
      items,
    );
  };

let parseNested = (json: Yojson.Safe.t, default: LegacyConfigurationValues.t) => {
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
      LegacyConfiguration.{default: configurationValues, perFiletype};
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
