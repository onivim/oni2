open Oni_Core;
open Utility;

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

let persist = store => {
  let entries =
    store.entries
    |> List.map((Entry({definition, value})) =>
         (definition.key, value |> definition.codec.encode)
       );

  let str = Json.Encode.encode_string(Json.Encode.obj, entries);

  let path =
    Filesystem.getStoreFolder()
    |> Result.map(storeFolder => Filename.concat(storeFolder, store.hash))
    |> ResultEx.flatMap(Filesystem.getOrCreateConfigFolder)
    |> Result.map(folder => Filename.concat(folder, "store.json"))
    |> Result.get_ok;

  let outChannel = open_out(path);
  Printf.fprintf(outChannel, "%s", str);
  close_out(outChannel);
};

let persistIfDirty = (store, state) => {
  let isDirty =
    List.exists(
      (Entry({definition, value})) =>
        !definition.codec.equal(value, definition.get(state)),
      store.entries,
    );

  if (isDirty) {
    List.iter((Entry({definition, _} as entry)) => entry.value = definition.get(state), store.entries);
    Console.log("-- persisiting");
    persist(store);
  };
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
