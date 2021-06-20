[@deriving show]
type t =
  | Current
  | Horizontal
  | Inactive
  | Vertical
      // `shouldReuse` specifies the behavior in case there is an existing
      // vertical split. If `shouldReuse` is `false`, a new split will always
      // be created. If `shouldReuse` is `true`, the available existing
      // split will be used.
      ({shouldReuse: bool})
  | NewTab;
