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
  let hash =
    fun
    | "global" => "global"
    | value => value |> Hashtbl.hash |> Printf.sprintf("%x");
};

module Schema = {
  module Codec = {
    type t('value) = {
      equal: ('value, 'value) => bool,
      encode: 'value => Json.t,
      decode: Json.decoder('value),
    };

    let custom = (~equal, ~encode, ~decode) => {equal, encode, decode};

    module Builtins = {
      let bool = {
        equal: Bool.equal,
        encode: Json.Encode.bool,
        decode: Json.Decode.bool,
      };
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
    };
  };

  include Codec.Builtins;

  type item('state, 'value) = {
    key: string,
    default: 'value,
    codec: Codec.t('value),
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
};

module Store = {
  type entry('state) =
    | Entry({
        definition: Schema.item('state, 'value),
        mutable value: 'value,
      })
      : entry('state);

  let entry = definition => Entry({definition, value: definition.default});

  type t('state) = {
    name: string,
    hash: string,
    filePath: string,
    entries: list(entry('state)),
  };

  let instantiate = (name, entries) => {
    let hash = Internal.hash(name);
    let filePath =
      Filesystem.getStoreFolder()
      |> Result.map(storeFolder => Filename.concat(storeFolder, hash))
      |> ResultEx.flatMap(Filesystem.getOrCreateConfigFolder)
      |> Result.map(folder => Filename.concat(folder, "store.json"))
      |> Result.get_ok;
    let store = {name, hash, filePath, entries: entries()};

    switch (Yojson.Safe.from_file(filePath)) {
    | json =>
      switch (Json.Decode.(decode_value(key_value_pairs(value), json))) {
      | Ok(persistedEntries) =>
        List.iter(
          (Entry({definition, _} as entry)) =>
            switch (List.assoc_opt(definition.key, persistedEntries)) {
            | Some(value) =>
              switch (
                Json.Decode.decode_value(definition.codec.decode, value)
              ) {
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

    let outChannel = open_out(store.filePath);
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

  let get = (definition: Schema.item(_), store) => {
    let Entry({value, definition: {pipe, _}, _}) =
      List.find(
        (Entry({definition: this, _})) => this.key == definition.key,
        store.entries,
      );

    Pipe.send(pipe, definition.pipe, value) |> Option.get;
  };
};

module Global = {
  open Oni_Model.State;
  open Schema;

  let version =
    define("version", string, BuildInfo.commitId, _ => BuildInfo.commitId);
  let workspace =
    define("workspace", option(string), None, state =>
      Some(state.workspace.workingDirectory)
    );

  let store =
    Store.instantiate("global", () =>
      [Store.entry(version), Store.entry(workspace)]
    );
};

module Workspace = {
  open Schema;

  type state = (Oni_Model.State.t, Revery.Window.t);

  let windowX =
    define("windowX", option(int), None, ((_state, window)) =>
      Some(Revery.Window.getPosition(window) |> fst)
    );
  let windowY =
    define("windowY", option(int), None, ((_state, window)) =>
      Some(Revery.Window.getPosition(window) |> snd)
    );
  let windowWidth =
    define("windowWidth", int, 800, ((_state, window)) =>
      Revery.Window.getRawSize(window).width
    );
  let windowHeight =
    define("windowHeight", int, 600, ((_state, window)) =>
      Revery.Window.getRawSize(window).height
    );
  let windowMaximized =
    define("windowMazimized", bool, false, ((_state, window)) =>
      Revery.Window.isMaximized(window)
    );

  let instantiate = path =>
    Store.instantiate(path, () =>
      [
        Store.entry(windowX),
        Store.entry(windowY),
        Store.entry(windowWidth),
        Store.entry(windowHeight),
        Store.entry(windowMaximized),
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
