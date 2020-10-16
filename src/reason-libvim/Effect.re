type t =
  | Goto(Goto.effect)
  | TabPage(TabPage.effect)
  | Format(Format.effect)
  | ModeChanged(Mode.t)
  | SettingChanged(Setting.t)
  | ColorSchemeChanged(option(string))
  | MacroRecordingStarted({register: char})
  | MacroRecordingStopped({
      register: char,
      value: option(string),
    })
  | Scroll({
      count: int,
      direction: Scroll.direction,
    });
