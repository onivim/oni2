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
};

let closeOnExit = ({closeOnExit, _}) => closeOnExit;

let initial = (~id, ~launchConfig) => {
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
};

