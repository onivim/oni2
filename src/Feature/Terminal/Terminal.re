type t = {
  id: int,
  launchConfig: Exthost.ShellLaunchConfig.t,
  rows: int,
  columns: int,
  pid: option(int),
  title: option(string),
  screen: EditorTerminal.Screen.t,
  cursor: EditorTerminal.Cursor.t,
  closeOnExit: bool,
  scrollY: float,
  font: EditorTerminal.Font.t,
};

[@deriving show]
type msg =
  | MouseWheelScrolled({deltaY: float});

let closeOnExit = ({closeOnExit, _}) => closeOnExit;

let initial = (~font, ~id, ~launchConfig) => {
  id,
  launchConfig,
  rows: 40,
  columns: 40,
  pid: None,
  title: None,
  screen: EditorTerminal.Screen.initial,
  cursor: EditorTerminal.Cursor.initial,
  closeOnExit: true,
  scrollY: 0.,
  font,
};

let setFont = (~font, terminal) => {...terminal, font};

let totalHeight = ({rows, font, _}) => {
  float(rows) *. EditorTerminal.Font.(font.lineHeight);
};

let availableScrollY = ({screen, rows, _}) => {};

let scroll = (~deltaY, terminal) => {
  ...terminal,
  scrollY: terminal.scrollY +. deltaY,
};

let update = (msg, terminal) => {
  switch (msg) {
  | MouseWheelScrolled({deltaY}) => scroll(~deltaY, terminal)
  };
};

