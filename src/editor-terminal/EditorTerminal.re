module Cursor = Cursor;
module Font = Font;
module Screen = Screen;
module Theme = Theme;

type effect =
  | ScreenResized(Screen.t)
  | ScreenUpdated(Screen.t)
  | TermPropChanged(Vterm.TermProp.t)
  | CursorMoved(Cursor.t)
  | Output(string);

type t = {
  screen: ref(Screen.t),
  vterm: Vterm.t,
  cursor: ref(Cursor.t),
};

type unsubscribe = unit => unit;

let make =
    (
      ~scrollBackSize=1000,
      ~rows: int,
      ~columns: int,
      ~onEffect as dispatch,
      (),
    ) => {
  let cursor = ref(Cursor.initial);
  let vterm = Vterm.make(~rows, ~cols=columns);
  let screen = ref(Screen.make(~vterm, ~scrollBackSize, ~rows, ~columns));
  Vterm.setUtf8(~utf8=true, vterm);
  Vterm.Screen.setAltScreen(~enabled=true, vterm);

  Vterm.setOutputCallback(~onOutput=str => {dispatch(Output(str))}, vterm);

  Vterm.Screen.setMoveCursorCallback(
    ~onMoveCursor=
      (newPos, _oldPos, _visible) => {
        let newCursor =
          Cursor.{
            ...cursor^,
            row: newPos.row,
            column: newPos.col,
            visible: true,
          };
        dispatch(CursorMoved(newCursor));
        cursor := newCursor;
      },
    vterm,
  );

  Vterm.Screen.setResizeCallback(
    ~onResize=
      ({rows, cols}) => {
        screen := Screen.resize(~rows, ~columns=cols, screen^);
        dispatch(ScreenResized(screen^));
      },
    vterm,
  );

  Vterm.Screen.setScrollbackPushCallback(
    ~onPushLine=
      cells => {
        screen := Screen.Internal.pushScrollback(~cells, screen^);
        dispatch(ScreenUpdated(screen^));
      },
    vterm,
  );
  Vterm.Screen.setScrollbackPopCallback(
    ~onPopLine=
      cells => {
        screen := Screen.Internal.pushScrollback(~cells, screen^);
        dispatch(ScreenUpdated(screen^));
      },
    vterm,
  );

  Vterm.Screen.setDamageCallback(
    ~onDamage=
      ({startRow, startCol, endRow, endCol}: Vterm.Rect.t) => {
        let _screen = screen^;
        for (col in startCol to endCol - 1) {
          for (row in startRow to endRow - 1) {
            Screen.Internal.markDamaged(_screen, row, col);
          };
        };
        screen := Screen.Internal.bumpDamageCounter(_screen);
        dispatch(ScreenUpdated(screen^));
      },
    vterm,
  );

  Vterm.Screen.setTermPropCallback(
    ~onSetTermProp=
      prop => {
        switch (prop) {
        | Vterm.TermProp.CursorShape(shape) =>
          cursor := Cursor.{...cursor^, shape}
        | _ => ()
        };
        dispatch(TermPropChanged(prop));
      },
    vterm,
  );

  {screen, vterm, cursor};
};

let resize = (~rows, ~columns, {vterm, screen, _}) => {
  screen := Screen.resize(~rows, ~columns, screen^);
  Vterm.setSize(~size={rows, cols: columns}, vterm);

  // After the size changed - re-get all the cells
  let _screen = screen^;
  for (col in 0 to columns - 1) {
    for (row in 0 to rows - 1) {
      Screen.Internal.markDamaged(_screen, row, col);
    };
  };
  screen := Screen.Internal.bumpDamageCounter(_screen);
};

let write = (~input: string, {vterm, _}) => {
  Vterm.write(~input, vterm) |> (ignore: int => unit);
};

let input = (~modifier=Vterm.None, ~key: Vterm.key, {vterm, _}) => {
  Vterm.Keyboard.input(vterm, key, modifier);
};

let cursor = ({cursor, _}) => cursor^;
let screen = ({screen, _}) => screen^;

let render =
    (
      ~opacity=?,
      ~defaultForeground=?,
      ~defaultBackground=?,
      ~theme=Theme.default,
      ~scrollY,
      ~font,
      ~cursor,
      screen,
    ) =>
  <TerminalView
    ?opacity
    ?defaultForeground
    ?defaultBackground
    scrollY
    theme
    screen
    cursor
    font
  />;
