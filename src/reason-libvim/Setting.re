type value =
| String(string)
| Int(int);

type t = {
  fullName: string,
  shortName: string,
  value: value,
};
