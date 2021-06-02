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
  // Whether the terminal should auto-scroll when content is added..
  autoScroll: bool,
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
  // Whether the terminal should auto-scroll when content is added..
  autoScroll: true,
  font,
};

let setFont = (~font, terminal) => {...terminal, font};

let viewportHeight = ({rows, font, _}) => {
  float(rows) *. EditorTerminal.Font.(font.lineHeight);
};

let totalHeight = ({screen, font, _}) => {
  let totalRows = EditorTerminal.Screen.getTotalRows(screen);

  float(totalRows) *. EditorTerminal.Font.(font.lineHeight);
};

let availableScrollY = terminal => {
  max(0., totalHeight(terminal) -. viewportHeight(terminal));
};

let scrollToBottom = terminal => {
  let scrollY = availableScrollY(terminal);
  {...terminal, scrollY, autoScroll: true};
};

let scroll = (~deltaY, terminal) => {
  let candidateScrollY = terminal.scrollY +. deltaY;
  let availableScrollY = availableScrollY(terminal);

  let scrollY =
    if (candidateScrollY < 0.) {
      0.;
    } else if (candidateScrollY >= availableScrollY -. 1.0) {
      availableScrollY;
    } else {
      candidateScrollY;
    };

  let isCloseToEnd = candidateScrollY == availableScrollY;

  let autoScroll = isCloseToEnd ? true : terminal.autoScroll;

  {...terminal, autoScroll, scrollY};
};

let updateScreen = (~screen, ~cursor, terminal) => {
  let terminal' = {...terminal, screen, cursor};

  if (terminal.autoScroll) {
    terminal' |> scrollToBottom;
  } else {
    terminal';
  };
};

let update = (msg, terminal) => {
  switch (msg) {
  | MouseWheelScrolled({deltaY}) => scroll(~deltaY, terminal)
  };
};

