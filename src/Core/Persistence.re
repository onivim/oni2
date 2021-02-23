open Kernel;
open Utility;

module Log = (val Log.withNamespace("Oni2.Core.Persistence"));

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
        encode: Json.Encode.nullable(codec.encode),
        decode: Json.Decode.maybe(codec.decode),
      };
      let value = {
        equal: Yojson.Safe.equal,
        encode: Json.Encode.value,
        decode: Json.Decode.value,
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
    filePath: option(string),
    entries: list(entry('state)),
  };

  let read = store =>
    switch (store.filePath) {
    | Some(filePath) =>
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
      | exception exn =>
        // Other exceptions would be unexpected here, but could happen with extraordinary circumstances,
        // ie a corrupted store file: https://github.com/onivim/oni2/1766
        Log.error(Printexc.to_string(exn))
      }
    | None =>
      Log.warn("Unable to read store due to no path. See previous error.")
    };

  let instantiate = (~storeFolder=?, name, entries) => {
    let hash = Internal.hash(name);
    let storeFolderResult =
      switch (storeFolder) {
      | None => Filesystem.getStoreFolder()
      | Some(folder) => Ok(folder)
      };
    let maybeFilePath =
      storeFolderResult
      |> Result.map(storeFolder => FpExp.append(storeFolder, hash))
      |> ResultEx.flatMap(Filesystem.getOrCreateConfigFolder)
      |> Result.map(folder => FpExp.append(folder, "store.json"))
      |> Result.map(FpExp.toString)
      |> Result.fold(
           ~ok=Option.some,
           ~error=message => {
             Log.error("Unable to get store path: " ++ message);
             None;
           },
         );

    let store = {name, hash, filePath: maybeFilePath, entries: entries()};

    read(store);

    store;
  };

  let isDirty = (state, store) =>
    List.exists(
      (Entry({definition, value})) =>
        !definition.codec.equal(value, definition.get(state)),
      store.entries,
    );

  let update = (state, store) =>
    List.iter(
      (Entry({definition, _} as entry)) =>
        entry.value = definition.get(state),
      store.entries,
    );

  let write = store => {
    Log.debug("Writing store for " ++ store.name);

    let jsonBuffer =
      store.entries
      |> List.map((Entry({definition, value})) =>
           (definition.key, definition.codec.encode(value))
         )
      |> Json.Encode.encode_string(Json.Encode.obj)
      |> Luv.Buffer.from_string;

    let then_ = (~error="Failed", f) =>
      fun
      | Ok(value) => f(value)
      | Error(luverr) =>
        Log.errorf(m => m("%s: %s", error, Luv.Error.strerror(luverr)));

    switch (store.filePath) {
    | Some(filePath) =>
      Luv.File.open_(
        filePath,
        [`WRONLY, `CREAT, `TRUNC],
        then_(~error="Failed to open store", file =>
          Luv.File.write(
            file,
            [jsonBuffer],
            then_(~error="Failed to write store", _ => ()),
          )
        ),
      )
    | None =>
      Log.warn("Unable to write store due to no path. See previous error.")
    };
  };

  let persist = (state, store) =>
    if (isDirty(state, store)) {
      update(state, store);
      write(store);
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
