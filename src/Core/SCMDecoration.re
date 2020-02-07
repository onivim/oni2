[@deriving show]
type t = {
  handle: int, // provider handle
  tooltip: string,
  letter: string,
  color: string, // TODO: ThemeColor.t?
  source: string,
};
