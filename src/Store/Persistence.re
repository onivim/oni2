open Oni_Core;
open Utility;

module Log = (val Log.withNamespace("Oni2.Store.Persistence"));

module Pipe = {
  type t('a) = ref(option('a));

  let create = () => ref(None);

  let send = (inPipe, outPipe, data) => {
    inPipe := Some(data); // send
    let received = outPipe^; // receive
    inPipe := None; // reset
    received; // return
  };
};

module Internal = {
  let hash = value => value |> Hashtbl.hash |> Printf.sprintf("%x");
};

type codec('value) = {
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
  codec: codec('value),
  get: 'state => 'value,
  pipe: Pipe.t('value),
};

let define = (key, codec, default, get) => {
  key,
  codec,
  default,
  get,
  pipe: Pipe.create(),
};

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
  let store = {name, hash: Internal.hash(name), entries: entries()};

  let path =
    Filesystem.getStoreFolder()
    |> Result.map(storeFolder => Filename.concat(storeFolder, store.hash))
    |> ResultEx.flatMap(Filesystem.getOrCreateConfigFolder)
    |> Result.map(folder => Filename.concat(folder, "store.json"))
    |> Result.get_ok;

  switch (Yojson.Safe.from_file(path)) {
  | json =>
    switch (Json.Decode.(decode_value(key_value_pairs(value), json))) {
    | Ok(persistedEntries) =>
      List.iter(
        (Entry({definition, _} as entry)) =>
          switch (List.assoc_opt(definition.key, persistedEntries)) {
          | Some(value) =>
            switch (Json.Decode.decode_value(definition.codec.decode, value)) {
            | Ok(value) => entry.value = value
            | Error(error) =>
              let message = Json.Decode.string_of_error(error);
              Log.error("Error decoding store file: " ++ message);
              entry.value = definition.default;
            }
          | None => entry.value = definition.default
          },
        store.entries,
      )

    | Error(error) =>
      let message = Json.Decode.string_of_error(error);
      Log.error("Error parsing store file: " ++ message);
    }
  | exception (Sys_error(message)) =>
    // Most likely because the file doesn't exist, which is expected, but log it just in case.
    Log.debug("Unable to read store file: " ++ message)
  };

  store;
};

let persist = store => {
  Log.debug("Writing store for " ++ store.name);
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
    List.iter(
      (Entry({definition, _} as entry)) =>
        entry.value = definition.get(state),
      store.entries,
    );

    persist(store);
  };
};

let get = (definition, store) => {
  let Entry({value, definition: {pipe, _}, _}) =
    List.find(
      (Entry({definition: this, _})) => this.key == definition.key,
      store.entries,
    );

  Pipe.send(pipe, definition.pipe, value) |> Option.get;
};

module Global = {
  open Oni_Model.State;

  let version =
    define("version", string, BuildInfo.commitId, _ => BuildInfo.commitId);
  let workspace =
    define("workspace", option(string), None, state =>
      Some(state.workspace.workingDirectory)
    );

  let store =
    instantiate("global", () => [entry(version), entry(workspace)]);
};

module Workspace = {
  type state = (Oni_Model.State.t, Revery.Window.t);

  let windowX =
    define("windowX", int, 0, ((_state, window)) =>
      Revery.Window.getPosition(window) |> fst
    );
  let windowY =
    define("windowY", int, 0, ((_state, window)) =>
      Revery.Window.getPosition(window) |> snd
    );
  let windowWidth =
    define("windowWidth", int, 800, ((_state, window)) =>
      Revery.Window.getRawSize(window).width
    );
  let windowHeight =
    define("windowHeight", int, 600, ((_state, window)) =>
      Revery.Window.getRawSize(window).height
    );

  let instantiate = path =>
    instantiate(path, () =>
      [
        entry(windowX),
        entry(windowY),
        entry(windowWidth),
        entry(windowHeight),
      ]
    );

  let storeFor = {
    let stores = Hashtbl.create(10);

    path =>
      switch (Hashtbl.find_opt(stores, path)) {
      | Some(store) => store
      | None =>
        let store = instantiate(path);
        Hashtbl.add(stores, path, store);
        store;
      };
  };
};
