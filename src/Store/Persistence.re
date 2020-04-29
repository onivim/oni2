open Oni_Core;

module Internal = {
  let hash = value => value |> Hashtbl.hash |> Printf.sprintf("%x");
};

type codec('state, 'value) = {
  equal: ('value, 'value) => bool,
  encode: 'value => Json.t,
  decode: Json.decoder('value),
};

let custom = (~equal, ~encode, ~decode) => {equal, encode, decode};

let int = {
  equal: Int.equal,
  encode: Json.Encode.int,
  decode: Json.Decode.int,
};
let string = {
  equal: String.equal,
  encode: Json.Encode.string,
  decode: Json.Decode.string,
};
let option = codec => {
  equal: Option.equal(codec.equal),
  encode: Json.Encode.option(codec.encode),
  decode: Json.Decode.maybe(codec.decode),
};

type definition('state, 'value) = {
  key: string,
  default: 'value,
  codec: codec('state, 'value),
  get: 'state => 'value,
};

let define = (key, codec, default, get) => {key, codec, default, get};

type entry('state) =
  | Entry({
      definition: definition('state, 'value),
      mutable value: 'value,
    })
    : entry('state);

let entry = definition => Entry({definition, value: definition.default});

type store('state) = {
  name: string,
  hash: string,
  entries: list(entry('state)),
};

let instantiate = (name, entries) => {
  name,
  hash: Internal.hash(name),
  entries: entries(),
};

module Global = {
  open Oni_Model.State;

  let version =
    define("version", string, BuildInfo.commitId, _ => BuildInfo.commitId);
  let workspace =
    define("workspace", option(string), None, state =>
      Base.Option.map(~f=ws => ws.workingDirectory, state.workspace)
    );

  let store =
    instantiate("global", () => [entry(version), entry(workspace)]);
};
