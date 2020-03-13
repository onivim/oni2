/*
 * TerminalView.re
 *
 * Component for the 'terminal' buffer renderer
 */

open Revery;
open Revery.UI;
open Oni_Model;

module EditorMetrics = Feature_Editor.EditorMetrics;

module Colors = Feature_Terminal.Colors;
module Theme = Feature_Theme;

module Constants = {
  let scrollBarThickness = 6;
  let scrollTrackColor = Color.rgba(0.0, 0.0, 0.0, 0.4);
  let scrollThumbColor = Color.rgba(0.5, 0.5, 0.5, 0.4);
};

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
    | 0 => Colors.ansiBlack.get(theme)
    | 1 => Colors.ansiRed.get(theme)
    | 2 => Colors.ansiGreen.get(theme)
    | 3 => Colors.ansiYellow.get(theme)
    | 4 => Colors.ansiBlue.get(theme)
    | 5 => Colors.ansiMagenta.get(theme)
    | 6 => Colors.ansiCyan.get(theme)
    | 7 => Colors.ansiWhite.get(theme)
    | 8 => Colors.ansiBrightBlack.get(theme)
    | 9 => Colors.ansiBrightRed.get(theme)
    | 10 => Colors.ansiBrightGreen.get(theme)
    | 11 => Colors.ansiBrightYellow.get(theme)
    | 12 => Colors.ansiBrightBlue.get(theme)
    | 13 => Colors.ansiBrightMagenta.get(theme)
    | 14 => Colors.ansiBrightCyan.get(theme)
    | 15 => Colors.ansiBrightWhite.get(theme)
    // For 256 colors, fall back to defaults
    | idx => ReveryTerminal.Theme.default(idx);

  let defaultBackground = Colors.background.get(theme);
  let defaultForeground = Colors.foreground.get(theme);

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
          ~scrollBarThickness=Constants.scrollBarThickness,
          ~scrollBarThumb=Constants.scrollThumbColor,
          ~scrollBarBackground=Constants.scrollTrackColor,
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
