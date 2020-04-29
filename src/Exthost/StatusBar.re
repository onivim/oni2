[@deriving show]
type alignment =
  | Left
  | Right;

let stringToAlignment =
  fun
  | "0" => Left
  | "1" => Right
  | _ => Left;

[@deriving show]
type msg =
  | SetEntry({
      id: string,
      text: string,
      source: string,
      alignment,
      priority: int,
    });

let handle = (method, args: Yojson.Safe.t) => {
  switch (method, args) {
  | (
      "$setEntry",
      `List([
        `String(id),
        _,
        `String(source),
        `String(text),
        _,
        _,
        _,
        `String(alignment),
        `String(priority),
      ]),
    ) =>
    let alignment = stringToAlignment(alignment);
    let priority = int_of_string_opt(priority) |> Option.value(~default=0);
    Ok(SetEntry({id, source, text, alignment, priority}));
  | _ =>
    Error(
      "Unable to parse method: "
      ++ method
      ++ " with args: "
      ++ Yojson.Safe.to_string(args),
    )
  };
};
