open Utility;

type t =
  | String(string)
  | Int(int);

type decoder('a) = {run: t => result('a, string)};

type fontDescription = {
  fontFamily: string,
  height: option(float),
};

module Schema = {
  let succeed = v => {run: _ => Ok(v)};
  let fail = err => {run: _ => Error(err)};

  let string: decoder(string) = {
    run:
      fun
      | String(str) => Ok(str)
      | Int(_) => Error("Expected string."),
  };

  let bool = {
    run:
      fun
      | String(str) => Error("Expected bool, got string: " ++ str)
      | Int(1) => Ok(true)
      | Int(0) => Ok(false)
      | Int(v) => Error("Expected bool, got integer: " ++ string_of_int(v)),
  };

  let int = {
    run:
      fun
      | Int(v) => Ok(v)
      | String(str) =>
        switch (int_of_string_opt(str)) {
        | None => Error("Expected int, got string: " ++ str)
        | Some(i) => Ok(i)
        },
  };

  let map = (f, decoder) => {
    run: input => {
      decoder.run(input) |> Result.map(f);
    },
  };

  let and_then = (f, decoder) => {
    run: input => {
      decoder.run(input) |> ResultEx.flatMap(f);
    },
  };

  let font =
    string
    |> and_then(fontString => {
         let splitString = String.split_on_char(':', fontString);
         switch (splitString) {
         | [fontFamily, fontSize, ..._] =>
           Ok({
             fontFamily: StringEx.unquote(fontFamily),
             height: Float.of_string_opt(fontSize),
           })
         | [fontFamily] =>
           Ok({fontFamily: StringEx.unquote(fontFamily), height: None})
         | [] => Error("Unexpected format: " ++ fontString)
         };
       });
};

let decode_value = (decoder: decoder('a), v) => {
  decoder.run(v);
};

let decode_value_opt = (decoder: decoder('a), v) => {
  decoder.run(v) |> Result.to_option;
};
