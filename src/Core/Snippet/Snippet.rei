[@deriving show]
type segment =
  | Text(string)
  | Placeholder({
      index: int,
      contents: list(segment),
    })
  | Choice({
      index: int,
      choices: list(string),
    })
  | Variable({
      name: string,
      default: option(string),
    });

[@deriving show]
type t = list(list(segment));

let parse: string => result(t, string);
