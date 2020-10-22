open Oni_Core;

type environment =
  | Inherit
  | Additive(StringMap.t(string))
  | Strict(StringMap.t(string));

type t = {
  name: string,
  executable: string,
  arguments: list(string),
  env: environment,
};

module Internal = {
  let mapToJsonMap = map =>
    map
    |> StringMap.bindings
    |> List.map(((key, item)) => (key, `String(item)));
};

let to_yojson = ({name, executable, arguments, env}) => {
  let (strict, env') =
    switch (env) {
    | Inherit => (`Null, `Null)

    | Additive(map) => (`Bool(false), `Assoc(map |> Internal.mapToJsonMap))

    | Strict(map) => (`Bool(true), `Assoc(map |> Internal.mapToJsonMap))
    };

  let args = arguments |> List.map(s => `String(s));
  `Assoc([
    ("name", `String(name)),
    ("executable", `String(executable)),
    ("args", `List(args)),
    ("strictEnv", strict),
    ("env", env'),
  ]);
};
