[@deriving show]
type value =
| String(string)
| Int(int);

[@deriving show]
type t = {
  fullName: string,
  shortName: string,
  value: value,
};
