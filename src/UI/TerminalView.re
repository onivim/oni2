/*
 * TerminalView.re
 *
 * Component for the 'terminal' buffer renderer
 */

open Revery.UI;
open Oni_Model;

module EditorMetrics = Feature_Editor.EditorMetrics;

module Colors = Feature_Terminal.Colors;
module Theme = Feature_Theme;

module Styles = {
  let container = (metrics: EditorMetrics.t) =>
    Style.[
      position(`Relative),
      width(metrics.pixelWidth),
      height(metrics.pixelHeight),
    ];
};

let make =
    (
      ~metrics: EditorMetrics.t,
      ~terminal: Feature_Terminal.terminal,
      ~font: Service_Font.font,
      ~theme,
      (),
    ) => {
  let maybeFont = Revery.Font.load(font.fontFile) |> Stdlib.Result.to_option;

  let size = font.fontSize;
  let smoothing = font.smoothing;

  let onDimensionsChanged =
      (
        {height, width, _}: Revery.UI.NodeEvents.DimensionsChangedEventParams.t,
      ) => {
    // If we have a loaded font, figure out how many columns and rows we can show
    Option.iter(
      font => {
        let terminalFont = ReveryTerminal.Font.make(~size, font);
        let rows =
          float_of_int(height) /. terminalFont.lineHeight |> int_of_float;
        let columns =
          float_of_int(width) /. terminalFont.characterWidth |> int_of_float;

        GlobalContext.current().dispatch(
          Actions.Terminal(
            Feature_Terminal.Resized({id: terminal.id, rows, columns}),
          ),
        );
      },
      maybeFont,
    );
  };

  let terminalTheme =
    fun
    | 0 => theme#color(Colors.ansiBlack)
    | 1 => theme#color(Colors.ansiRed)
    | 2 => theme#color(Colors.ansiGreen)
    | 3 => theme#color(Colors.ansiYellow)
    | 4 => theme#color(Colors.ansiBlue)
    | 5 => theme#color(Colors.ansiMagenta)
    | 6 => theme#color(Colors.ansiCyan)
    | 7 => theme#color(Colors.ansiWhite)
    | 8 => theme#color(Colors.ansiBrightBlack)
    | 9 => theme#color(Colors.ansiBrightRed)
    | 10 => theme#color(Colors.ansiBrightGreen)
    | 11 => theme#color(Colors.ansiBrightYellow)
    | 12 => theme#color(Colors.ansiBrightBlue)
    | 13 => theme#color(Colors.ansiBrightMagenta)
    | 14 => theme#color(Colors.ansiBrightCyan)
    | 15 => theme#color(Colors.ansiBrightWhite)
    // For 256 colors, fall back to defaults
    | idx => ReveryTerminal.Theme.default(idx);

  let defaultBackground = theme#color(Colors.background);
  let defaultForeground = theme#color(Colors.foreground);

  let element =
    Option.map(
      font => {
        let {screen, cursor, _}: Feature_Terminal.terminal = terminal;
        let font = ReveryTerminal.Font.make(~smoothing, ~size, font);
        ReveryTerminal.render(
          ~defaultBackground,
          ~defaultForeground,
          ~theme=terminalTheme,
          ~cursor,
          ~font,
          screen,
        );
      },
      maybeFont,
    )
    |> Option.value(~default=React.empty);
  <View onDimensionsChanged style={Styles.container(metrics)}>
    element
  </View>;
};
