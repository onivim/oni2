type t =
  | Goto(Goto.effect)
  | TabPage(TabPage.effect)
  | Format(Format.effect)
  | ModeChanged(Mode.t);
