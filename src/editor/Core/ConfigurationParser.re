/*
 * ConfigurationParser.re
 *
 * Resilient parsing for Configuration
 */
open Configuration;
open LineNumber;

let parseBool = (json) => switch(json) {
| `Bool(v) => v
| _ => false
};

let parseLineNumberSetting = (json) => switch(json) {
| `String(v) => switch(v) {
    | "on" => On
    | "off" => Off
    | "relative" => Relative
    | _ => On
}
| _ => On
};

type parseFunction = (Configuration.t, Yojson.Safe.json) => Configuration.t;

type configurationTuple = (string, parseFunction);

let configurationParsers: list(configurationTuple) = [
 ("editor.lineNumbers", (s, v) => {...s, editorLineNumbers: parseLineNumberSetting(v)}),
 ("editor.minimap.enabled", (s, v) => {...s, editorMinimapEnabled: parseBool(v)}),

];

let keyToParser: Hashtbl.t(string, parseFunction) = List.fold_left((prev, cur) => {
    let (key, parser) = cur;
    Hashtbl.add(prev, key, parser);
    prev;
}, Hashtbl.create(100), configurationParsers);

let ofJson = json => {
    switch (json) {
    | `Assoc(items) => {
        Ok(List.fold_left((prev, cur) => {
            let (key, json) = cur;
            switch(Hashtbl.find_opt(keyToParser, key)) {
            | Some(v) => v(prev, json)
            | None => prev
            };

        }, Configuration.default, items));
    }
    | _ => Error("Incorrect JSON format for configuration")
    }
};

let ofString = str => {
    switch(
    str |> Yojson.Safe.from_string |> ofJson
    ) {
    | v => v
    | exception Yojson.Json_error(msg) => Error(msg);
    }
}
