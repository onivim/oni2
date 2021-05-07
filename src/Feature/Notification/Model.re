// MODEL

[@deriving show({with_path: false})]
type kind =
  | Info
  | Warning
  | Error;

[@deriving show({with_path: false})]
type notification = {
  id: int,
  kind,
  message: string,
  source: option(string),
  yOffset: float,
  // If `ephemeral` is true, don't store in notification list
  ephemeral: bool,
};
