[@deriving show]
type t =
  | Current
  | Horizontal
  | Vertical({shouldReuse: bool})
  | NewTab;
