[@deriving show({with_path: false})]
type t =
  | True
  | False
  | String(string)
  | Integer(int)
  | Real(float)
  // Date
  // Data
  | Array(list(t))
  | Dict(list((string, t)));

type decoder('a) = t => result('a, string);

let id = value => Ok(value);

let bool =
  fun
  | True => Ok(true)
  | False => Ok(false)
  | value => Error("Expected bool, got: " ++ show(value));

let string =
  fun
  | String(value) => Ok(value)
  | value => Error("Expected string, got: " ++ show(value));

let integer =
  fun
  | Integer(value) => Ok(value)
  | value => Error("Expected integer, got: " ++ show(value));

let real =
  fun
  | Real(value) => Ok(value)
  | value => Error("Expected real, got: " ++ show(value));

let array = decodeValue =>
  fun
  | Array(items) => {
      let rec loop = (values, acc) =>
        switch (values) {
        | [] => Ok(acc)
        | [value, ...rest] =>
          switch (decodeValue(value)) {
          | Ok(value) => loop(rest, [value, ...acc])
          | Error(message) => Error(message)
          }
        };

      loop(items, []);
    }
  | value => Error("Expected array, got: " ++ show(value));

let property = (key, decodeValue) =>
  fun
  | Dict(properties) as dict =>
    switch (List.assoc_opt(key, properties)) {
    | Some(value) => decodeValue(value)
    | None =>
      Error("Expected dict with property " ++ key ++ ", got " ++ show(dict))
    }
  | value => Error("Expected dict, got " ++ show(value));

type dictGetters = {
  required: 'a. (string, decoder('a)) => 'a,
  optional: 'a. (string, decoder('a)) => option('a),
  withDefault: 'a. (string, decoder('a), 'a) => 'a,
};

let dict = f =>
  fun
  | Dict(properties) as dict => {
      exception DecodeError(string);

      let get = (key, decodeValue) =>
        switch (List.assoc_opt(key, properties)) {
        | Some(value) =>
          switch (decodeValue(value)) {
          | Ok(value) => value
          | Error(message) =>
            raise(DecodeError(message ++ "\n\tat " ++ key))
          }
        | None =>
          raise(
            DecodeError(
              "Expected dict with property " ++ key ++ ", got " ++ show(dict),
            ),
          )
        };

      let getters = {
        required: get,
        optional: (key, decodeValue) =>
          switch (get(key, decodeValue)) {
          | value => Some(value)
          | exception (DecodeError(_)) => None
          },
        withDefault: (key, decodeValue, default) =>
          switch (get(key, decodeValue)) {
          | value => value
          | exception (DecodeError(_)) => default
          },
      };

      switch (f(getters)) {
      | value => Ok(value)
      | exception (DecodeError(message)) => Error(message)
      };
    }
  | value => Error("Expected dict, got " ++ show(value));

let assoc = decodeValue =>
  fun
  | Dict(properties) => {
      let rec loop = (values, acc) =>
        switch (values) {
        | [] => Ok(acc)
        | [(key, value), ...rest] =>
          switch (decodeValue(value)) {
          | Ok(value) => loop(rest, [(key, value), ...acc])
          | Error(message) => Error(message ++ "\n\tat " ++ key)
          }
        };

      loop(properties, []);
    }
  | value => Error("Expected dict, got " ++ show(value));

let oneOf = (decoders, value) => {
  let rec loop = (decoders, errors) =>
    switch (decoders) {
    | [] =>
      let errors =
        errors
        |> List.map(((name, message)) =>
             Printf.sprintf("\n%s:\n\t%s", name, message)
           )
        |> String.concat("\n");

      Error(
        "Expected one of several decoders to succeed, but got::\n" ++ errors,
      );

    | [(name, decode), ...rest] =>
      switch (decode(value)) {
      | Ok(value) => Ok(value)
      | Error(message) => loop(rest, [(name, message), ...errors])
      }
    };

  loop(decoders, []);
};

let map = (f, decode, value) => decode(value) |> Result.map(f);

let option = (decode, value) =>
  switch (decode(value)) {
  | Ok(value) => Ok(Some(value))
  | Error(_) => Ok(None)
  };
