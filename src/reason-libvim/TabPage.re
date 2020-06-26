type effect =
  | Goto(int)
  | Previous(int)
  | Next
  | Move(int)
  | Close(int)
  | CloseOther(int);
