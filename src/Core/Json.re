open EditorCoreTypes;
open Oniguruma;

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

  let regexp = {
    let str =
      string
      |> map(OnigRegExp.create)
      |> and_then(
           fun
           | Ok(regexp) => succeed(regexp)
           | Error(msg) => {
               fail(Printf.sprintf("Error %s parsing regex", msg));
             },
         );

    let dto =
      obj(({field, _}) => {
        field.required(
          "pattern",
          str,
          // TODO: Flags field?
        )
      });

    one_of([("string", str), ("dto", dto)]);
  };

  let%test "decode valid regexp" = {
    Yojson.Safe.from_string({|"abc"|})
    |> decode_value(regexp)
    |> Result.is_ok;
  };

  let%test "decode invalid regexp" = {
    Yojson.Safe.from_string({|"(invalid"|})
    |> decode_value(regexp)
    |> Result.is_error;
  };

  let default = default => map(Option.value(~default));
};

module Encode = {
  include Decoders_yojson.Safe.Encode;
};

type decoder('a) = Decode.decoder('a);
type encoder('a) = Encode.encoder('a);

module Error = {
  type t = {
    range: CharacterRange.t,
    message: string,
  };

  let ofString = {
    open Oniguruma;
    let yojsonRegExp =
      OnigRegExp.create("Line ([0-9]+), bytes ([0-9]+)-([0-9]+):\n(.*)$");

    msg => {
      yojsonRegExp
      |> Result.to_option
      |> Utility.OptionEx.flatMap(regex => {
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
             let start =
               CharacterPosition.{
                 line: EditorCoreTypes.LineNumber.(zero + line - 1),
                 character: CharacterIndex.(zero + startByte),
               };
             let stop =
               CharacterPosition.{
                 line: EditorCoreTypes.LineNumber.(zero + line - 1),
                 character: CharacterIndex.(zero + endByte),
               };
             Some({range: CharacterRange.{start, stop}, message});
           };
         });
    };
  };
};
