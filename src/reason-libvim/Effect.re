type t =
  | Goto(Goto.effect)
  | TabPage(TabPage.effect)
  | Format(Format.effect)
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
    })
  | SearchStringChanged(option(string))
  | SearchClearHighlights
  | Map(Mapping.t)
  | Unmap({
      mode: Mapping.mode,
      keys: option(string),
    })
  | Clear(Clear.t)
  | Output({
      cmd: string,
      output: option(string),
    })
  | WindowSplit(Split.t);
