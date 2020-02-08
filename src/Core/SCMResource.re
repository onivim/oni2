[@deriving show({with_path: false})]
type t = {
  handle: int,
  uri: Uri.t,
  icons: list(string),
  tooltip: string,
  strikeThrough: bool,
  faded: bool,
  source: option(string),
  letter: option(string),
  color: option(string),
};
