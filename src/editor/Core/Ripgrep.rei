[@deriving show]
type t = {search: (string, list(string) => unit) => unit};

let make: string => t;
