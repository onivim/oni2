/*
 * ConfigurationParser.re
 *
 * Resilient parsing for Configuration
 */
open Configuration;
open LineNumber;

let parseBool = json =>
  switch (json) {
  | `Bool(v) => v
  | _ => false
  };

let parseInt = json =>
  switch (json) {
  | `Int(v) => v
  | _ => 0
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

let parseTablineMode = json =>
switch (json) {
| `String(v) =>
    switch(v) {
    | "buffers" => Buffers
    | "tabs" => Tabs
    | "hybrid" => Hybrid
    | _ => Buffers
    }
| _ => Buffers
};

let parseRenderWhitespace = json => switch(json) {
| `String(v) => {
    switch(v) {
    | "all" => All
    | "boundary" => Boundary
    | "none" => None
    | _ => All
    }
}
| _ => All
};

let parseString = json => switch(json) {
| `String(v) => v
| _ => ""
};

type parseFunction = (Configuration.t, Yojson.Safe.json) => Configuration.t;

type configurationTuple = (string, parseFunction);

let configurationParsers: list(configurationTuple) = [
  (
    "editor.lineNumbers",
    (s, v) => {...s, editorLineNumbers: parseLineNumberSetting(v)},
  ),
  (
    "editor.minimap.enabled",
    (s, v) => {...s, editorMinimapEnabled: parseBool(v)},
  ),
  (
    "editor.minimap.showSlider",
    (s, v) => {...s, editorMinimapShowSlider: parseBool(v)},
  ),
  (
    "editor.tablineMode",
    (s, v) => {...s, editorTablineMode: parseTablineMode(v)},
  ),
  (
    "editor.insertSpaces",
    (s, v) => {...s, editorInsertSpaces: parseBool(v)},
  ),
  (
    "editor.indentSize",
    (s, v) => {...s, editorIndentSize: parseInt(v)},
  ),
  (
    "editor.tabSize",
    (s, v) => {...s, editorTabSize: parseInt(v)},
  ),
  (
    "editor.highlightActiveIndentGuide",
    (s, v) => {...s, editorHighlightActiveIndentGuide: parseBool(v)},
  ),
  (
    "editor.renderIndentGuides",
    (s, v) => {...s, editorRenderIndentGuides: parseBool(v)},
  ),
  (
    "editor.renderWhitespace",
    (s, v) => {...s, editorRenderWhitespace: parseRenderWhitespace(v)},
  ),
  (
    "workbench.iconTheme",
    (s, v) => {...s, workbenchIconTheme: parseString(v)},
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

let ofJson = json => {
  switch (json) {
  | `Assoc(items) =>
    Ok(
      List.fold_left(
        (prev, cur) => {
          let (key, json) = cur;
          switch (Hashtbl.find_opt(keyToParser, key)) {
          | Some(v) => v(prev, json)
          | None => prev
          };
        },
        Configuration.default,
        items,
      ),
    )
  | _ => Error("Incorrect JSON format for configuration")
  };
};

let ofString = str => {
  switch (str |> Yojson.Safe.from_string |> ofJson) {
  | v => v
  | exception (Yojson.Json_error(msg)) => Error(msg)
  };
};

let ofFile = filePath => Yojson.Safe.from_file(filePath) |> ofJson;
