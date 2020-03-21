// TODO;
let count = _ => 0;

type keybinding = {
  key: string,
  command: string,
  condition: WhenExpr.t,
};

module Keybinding = {
  type t = keybinding;

  let parseSimple =
    fun
    | `String(s) => WhenExpr.Defined(s)
    | _ => WhenExpr.Value(False);

  let parseAndExpression =
    fun
    | `String(expr) => WhenExpr.Defined(expr)
    | `List(andExpressions) =>
      WhenExpr.And(List.map(parseSimple, andExpressions))
    | _ => WhenExpr.Value(False);

  let condition_of_yojson =
    fun
    | `List(orExpressions) =>
      Ok(WhenExpr.Or(List.map(parseAndExpression, orExpressions)))
    | `String(v) => Ok(WhenExpr.parse(v))
    | `Null => Ok(WhenExpr.Value(True))
    | _ => Error("Expected string for condition");

  let of_yojson = (json: Yojson.Safe.t) => {
    let key = Yojson.Safe.Util.member("key", json);
    let command = Yojson.Safe.Util.member("command", json);
    let condition =
      Yojson.Safe.Util.member("when", json) |> condition_of_yojson;

    switch (condition) {
    | Ok(condition) =>
      switch (key, command) {
      | (`String(key), `String(command)) => Ok({key, command, condition})
      | (`String(_), _) => Error("Command must be a string")
      | (_, `String(_)) => Error("Key must be a string")
      | _ => Error("Binding must specify key and command strings")
      }

    | Error(msg) => Error(msg)
    };
  };
};

module Internal = {
  let of_yojson_with_errors:
    list(Yojson.Safe.t) => (list(Keybinding.t), list(string)) =
    bindingsJson => {
      let parsedBindings =
        bindingsJson
        // Parse each binding
        |> List.map(Keybinding.of_yojson);

      // Get errors from individual keybindings, but don't let them stop parsing
      let errors =
        List.filter_map(
          keyBinding =>
            switch (keyBinding) {
            | Ok(_) => None
            | Error(msg) => Some(msg)
            },
          parsedBindings,
        );

      // Get valid bindings now
      let bindings =
        List.filter_map(
          keyBinding =>
            switch (keyBinding) {
            | Ok(v) => Some(v)
            | Error(_) => None
            },
          parsedBindings,
        );

      (bindings, errors);
    };
};

module Input = EditorInput.Make({
  type context = Hashtbl.t(string, bool);
  type payload = string;
});

type effect = 
| Command(string)
| Unhandled(EditorInput.key);

let empty = Input.empty;

type t = Input.t;

let _keyToVimString = (key: EditorInput.key) => {
  let name = ref(Sdl2.Keycode.getName(key.keycode));

  if (key.modifiers.alt) {
    name := "A-" ++ name^;
  };

  if (key.modifiers.control) {
    name := "C-" ++ name^;
  };

  if (key.modifiers.shift) {
    name := "S-" ++ name^;
  };

  if (key.modifiers.meta) {
    name := "D-" ++ name^;
  };

  let wrapIfLong = str =>
    if (String.length(str) > 1) {
      "<" ++ str ++ ">";
    } else {
      str;
    };

  let convertSdlName =
    fun
    | "<ESCAPE>" => "<ESC>"
    | "<RETURN>" => "<CR>"
    | v => v;

  name^ |> String.uppercase_ascii |> wrapIfLong |> convertSdlName;
};


let mapEffect = fun
| Input.Execute(cmd) => Command(cmd)
| Input.Unhandled(key) => Unhandled(key);

let mapEffects = List.map(mapEffect);

let keyDown = (~context, ~key, bindings) => {
  let (bindings, effects) = Input.keyDown(~context, key, bindings);

  let mappedEffects = mapEffects(effects);
  (bindings, mappedEffects);
};

// Old version of keybindings - the legacy format:
// { bindings: [ ..bindings. ] }
module Legacy = {
  let upgrade: list(Keybinding.t) => list(Keybinding.t) =
    keys => {
      let upgradeBinding = (binding: Keybinding.t) => {
        switch (binding.command) {
        | "quickOpen.open" => {
            ...binding,
            command: "workbench.action.quickOpen",
          }
        | "commandPalette.open" => {
            ...binding,
            command: "workbench.action.showCommands",
          }
        | _ => binding
        };
      };

      keys |> List.map(upgradeBinding);
    };
};

let strToSdl = fun
| "ESC" => "Escape"
| "CR" => "Enter"
| "UP" => "Up"
| "DOWN" => "Down"
| "LEFT" => "Left"
| "RIGHT" => "Right"
| "TAB" => "Tab"
| str => str;

let wrap = (f, s) => {
  prerr_endline ("S: " ++ s);
  let ret = f(s);
  prerr_endline ("RESULT: " ++ ret);
  ret;
}

let codeToOpt = fun
| 0 => None
| x => Some(x);

let getKeycode = keycodeStr => {
  keycodeStr
  |> wrap(strToSdl)
  |> Sdl2.Keycode.ofName
  |> codeToOpt;
};

let getScancode = scancodeStr =>  {
  scancodeStr
  |> strToSdl
  |> Sdl2.Scancode.ofName
  |> codeToOpt;
};

let addBinding = ({key, command, condition}, bindings) => {

  let evaluateCondition = (whenExpr, context) => {
    let getValue = v =>
      switch (Hashtbl.find_opt(context, v)) {
      | Some(true) => WhenExpr.Value.True
      | Some(false)
      | None => WhenExpr.Value.False
      };
  
    WhenExpr.evaluate(whenExpr, getValue);
  };

  let matchers = EditorInput.Matcher.parse(
     ~getKeycode,
     ~getScancode,
     key,
  );

  matchers
  |> Stdlib.Result.map(m => {
    let (bindings, _id) = Input.addBinding(
      m,
      evaluateCondition(condition),
      command,
      bindings,
    );
    bindings
  });
};

let evaluateBindings = (bindings: list(keybinding), errors) => {

  let rec loop = (bindings, errors, currentBindings) => {
  switch (bindings) {
  | [hd, ...tail] =>
      switch(addBinding(hd, currentBindings)) {
      | Ok(newBindings) => loop(tail, errors, newBindings);
      | Error(msg) =>
      prerr_endline ("FAIL: " ++ msg);
      failwith("ohno");
      loop(tail, [msg, ...errors], currentBindings);
      }
  | [] => (currentBindings, errors)
  }
  };

  loop(bindings, errors, Input.empty);
  
}

let of_yojson_with_errors = (~default=[], json) => {
  let previous =
    switch (json) {
    // Current format:
    // [ ...bindings ]
    | `List(bindingsJson) =>
      let res = bindingsJson |> Internal.of_yojson_with_errors;
      Ok(res);
    // Legacy format:
    // { bindings: [ ..bindings. ] }
    | `Assoc([("bindings", `List(bindingsJson))]) =>
      let res = bindingsJson |> Internal.of_yojson_with_errors;

      Ok(res)
      |> Stdlib.Result.map(((bindings, errors)) => {
           (Legacy.upgrade(bindings), errors)
         });
    | _ => Error("Unable to parse keybindings - not a JSON array.")
    };

  previous
  |> Stdlib.Result.map(((bindings, errors)) => {
       let combinedBindings = default @ bindings;
       evaluateBindings(combinedBindings, errors);
     });
};
