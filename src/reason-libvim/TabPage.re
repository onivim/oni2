[@deriving show({with_path: false})]
type effect =
  | Goto(int)
  | GotoRelative(int)
  | Move(int)
  | MoveRelative(int)
  | Close(int)
  | CloseRelative(int)
  | Only(int)
  | OnlyRelative(int);
