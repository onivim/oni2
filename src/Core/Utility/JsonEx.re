/*
  update method adapted from: https://github.com/ocaml-community/yojson/issues/54
 */
let update = (key, f, json) => {
  let rec update_json_obj =
    fun
    | [] =>
      switch (f(None)) {
      | None => []
      | Some(v) => [(key, v)]
      }
    | [(k, v) as m, ...tail] as original =>
      if (String.equal(k, key)) {
        switch (f(Some(v))) {
        | None => update_json_obj(tail)
        | Some(v') =>
          if (v' == v) {
            original;
          } else {
            [(k, v'), ...tail];
          }
        };
      } else {
        [m, ...update_json_obj(tail)];
      };

  switch (json) {
  | `Assoc(items) => `Assoc(update_json_obj(items))
  | _ => json
  };
};

let from_file = file =>
  try(Ok(Yojson.Safe.from_file(file))) {
  | e => Error(Printexc.to_string(e))
  };

let getKeys = json => {
  let rec loop = (curr, json) => {
    switch (json) {
    | `Assoc(items) =>
      items
      |> List.map(item => {
           let (key, v) = item;
           let prefix = curr == "" ? key : curr ++ "." ++ key;
           loop(prefix, v);
         })
      |> List.flatten
    | _ => [curr]
    };
  };

  loop("", json);
};

let parseYojsonErrorMessage = {
  open Oniguruma;
  let yojsonRegExp =
    OnigRegExp.create("Line ([0-9]+), bytes ([0-9]+)-([0-9]+):\n(.*)$");

  msg => {
    yojsonRegExp
    |> Result.to_option
    |> OptionEx.flatMap(regex => {
         let matches = regex |> OnigRegExp.search(msg, 0);
         if (Array.length(matches) < 5) {
           None;
         } else {
           let line = OnigRegExp.Match.getText(matches[1]) |> int_of_string;
           let startByte =
             OnigRegExp.Match.getText(matches[2]) |> int_of_string;
           let endByte =
             OnigRegExp.Match.getText(matches[3]) |> int_of_string;
           let message = OnigRegExp.Match.getText(matches[4]);
           Some((line, startByte, endByte, message));
         };
       });
  };
};

let explode_key = String.split_on_char('.');
/*
 [explode(json)] takes a JSON structure like:
 [{ "a.b.c": 1}]

 and converts it to:
 [{"a": { "b": { "c": 1 }}}]
 */
let explode = json => {
  let rec expand_item = (currJson, keys, jsonValue) => {
    switch (keys) {
    | [key] => update(key, _ => Some(jsonValue), currJson)
    | [key, ...remaining] =>
      update(
        key,
        fun
        | None => Some(expand_item(`Assoc([]), remaining, jsonValue))
        | Some(json) => Some(expand_item(json, remaining, jsonValue)),
        currJson,
      )

    | [] => jsonValue // Shouldn't hit this case...
    };
  }
  and expand_items = items => {
    List.fold_left(
      (acc, curr) => {
        let (key, jsonValue) = curr;

        let explodedKey = explode_key(key);
        expand_item(acc, explodedKey, loop(jsonValue));
      },
      `Assoc([]),
      items,
    );
  }
  and loop = json => {
    switch (json) {
    | `Assoc(items) => expand_items(items)
    | v => v
    };
  };

  loop(json);
};
