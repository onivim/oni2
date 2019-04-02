type t = {
  category: option(string),
  name: string,
  command: unit => unit,
  icon: option(string),
};
