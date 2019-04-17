/*
 * ConfigurationParser.re
 *
 * Resilient parsing for Configuration
 */

let ofJson = json => {
    switch (json) {
    | `Assoc(_) => {
        Ok(Configuration.default)
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
