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

let ofString = str => str |> Yojson.Safe.from_string |> ofJson;
