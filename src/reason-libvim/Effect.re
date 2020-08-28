type t =
  | Goto(Goto.effect)
  | TabPage(TabPage.effect)
  | Format(Format.effect)
  | ModeChanged(Mode.t)
  | SettingChanged(Setting.t)
  | MacroRecordingStarted({register: char})
  | MacroRecordingStopped({
      register: char,
      value: option(string),
    });
