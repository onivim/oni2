type t = Yojson.Safe.json;

module Decode = {
  open Decoders;

  module Json_decodeable: Decode.Decodeable with type value = Yojson.Safe.json = {
    type value = Yojson.Safe.json;
    let pp = (fmt, json) =>
      Format.fprintf(fmt, "@[%s@]", Yojson.Safe.pretty_to_string(json));

    let of_string: string => result(value, string) = (
      string =>
        try(Ok(Yojson.Safe.from_string(string))) {
        | Yojson.Json_error(msg) => Error(msg)
        }:
        string => result(value, string)
    );

    let of_file = file =>
      try(Ok(Yojson.Safe.from_file(file))) {
      | e => Error(Printexc.to_string(e))
      };

    let get_string =
      fun
      | `String(value) => Some(value)
      | _ => None;

    let get_int =
      fun
      | `Int(value) => Some(value)
      | _ => None;

    let get_float =
      fun
      | `Float(value) => Some(value)
      | `Int(value) => Some(float_of_int(value))
      | _ => None;

    let get_bool =
      fun
      | `Bool(value) => Some(value)
      | _ => None;

    let get_null =
      fun
      | `Null => Some()
      | _ => None;

    let get_list: value => option(list(value)) = (
      fun
      | `List(l) => Some(l)
      | _ => None:
        value => option(list(value))
    );

    let get_key_value_pairs: value => option(list((value, value))) = (
      fun
      | `Assoc(assoc) =>
        Some(List.map(((key, value)) => (`String(key), value), assoc))
      | _ => None:
        value => option(list((value, value)))
    );

    let to_list = values => `List(values);
  };

  include Decode.Make(Json_decodeable);

  type objGetters = {
    field: getters(string),
    at: getters(list(string)),
    whatever: 'a. decoder('a) => 'a,
  }
  and getters('arg) = {
    optional: 'a. ('arg, decoder('a)) => option('a),
    required: 'a. ('arg, decoder('a)) => 'a,
    withDefault: 'a. ('arg, 'a, decoder('a)) => 'a,
    monadic: 'a. ('arg, decoder('a)) => decoder('a),
  };

  let obj = f =>
    Decode.{
      run: json => {
        exception InternalDecodeError(Decode.exposed_error(t));

        let fieldGetters = {
          optional: (name, valueDecoder) =>
            switch (field(name, value).run(json)) {
            | Ok(json) =>
              switch (valueDecoder.run(json)) {
              | Ok(value) => Some(value)
              | Error(err) => raise(InternalDecodeError(err))
              }

            | Error(_) => None
            },

          required: (name, valueDecoder) =>
            switch (field(name, valueDecoder).run(json)) {
            | Ok(value) => value
            | Error(err) => raise(InternalDecodeError(err))
            },

          withDefault: (name, default, valueDecoder) =>
            switch (field(name, value).run(json)) {
            | Ok(json) =>
              switch (valueDecoder.run(json)) {
              | Ok(value) => value
              | Error(err) => raise(InternalDecodeError(err))
              }

            | Error(_) => default
            },
          monadic: field,
        };

        let atGetters = {
          optional: (names, valueDecoder) =>
            switch (at(names, value).run(json)) {
            | Ok(json) =>
              switch (valueDecoder.run(json)) {
              | Ok(value) => Some(value)
              | Error(err) => raise(InternalDecodeError(err))
              }

            | Error(_) => None
            },

          required: (names, valueDecoder) =>
            switch (at(names, valueDecoder).run(json)) {
            | Ok(value) => value
            | Error(err) => raise(InternalDecodeError(err))
            },

          withDefault: (names, default, valueDecoder) =>
            switch (at(names, value).run(json)) {
            | Ok(json) =>
              switch (valueDecoder.run(json)) {
              | Ok(value) => value
              | Error(err) => raise(InternalDecodeError(err))
              }

            | Error(_) => default
            },
          monadic: at,
        };

        let getters = {
          field: fieldGetters,
          at: atGetters,
          whatever: decoder =>
            switch (decoder.run(json)) {
            | Ok(value) => value
            | Error(error) => raise(InternalDecodeError(error))
            },
        };

        switch (f(getters)) {
        | value => Ok(value)
        | exception (InternalDecodeError(error)) => Error(error)
        };
      },
    };

  let default = default => map(Option.value(~default));
};

module Encode = {
  include Decoders_yojson.Safe.Encode;
};

type decoder('a) = Decode.decoder('a);
type encoder('a) = Encode.encoder('a);
