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

let to_yojson = ({name, executable, arguments, env}) => {
  let (strict, env') = switch(env) {
  | Inherit => (`Null, `Null)
  | Additive(map) => 
    let outEnv = map
    |> StringMap.bindings
    |> List.map(((key, item)) => (key, `String(item)));
    (`Bool(false), `Assoc(outEnv))
  | Strict(map) =>
    let outEnv = map
    |> StringMap.bindings
    |> List.map(((key, item)) => (key, `String(item)));
    (`Bool(true), `Assoc(outEnv));
  };
  let args = arguments |> List.map(s => `String(s));
  `Assoc([
    ("name", `String(name)),
    ("executable", `String(executable)),
    ("args", `List(args)),
    ("strictEnv", strict),
    ("env", env')
  ]);
};
