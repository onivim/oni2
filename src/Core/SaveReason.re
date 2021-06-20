[@deriving show]
type t =
  | UserInitiated({allowFormatting: bool})
  | AutoSave;
