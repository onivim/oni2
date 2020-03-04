/*
 * TerminalView.re
 *
 * Component for the 'terminal' buffer renderer
 */

open Revery;
open Revery.UI;
open Oni_Core;
open Oni_Model;

module EditorMetrics = Feature_Editor.EditorMetrics;

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
      ~theme: Theme.t,
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
    | 0 => theme.terminalAnsiBlack
    | 1 => theme.terminalAnsiRed
    | 2 => theme.terminalAnsiGreen
    | 3 => theme.terminalAnsiYellow
    | 4 => theme.terminalAnsiBlue
    | 5 => theme.terminalAnsiMagenta
    | 6 => theme.terminalAnsiCyan
    | 7 => theme.terminalAnsiWhite
    | 8 => theme.terminalAnsiBrightBlack
    | 9 => theme.terminalAnsiBrightRed
    | 10 => theme.terminalAnsiBrightGreen
    | 11 => theme.terminalAnsiBrightYellow
    | 12 => theme.terminalAnsiBrightBlue
    | 13 => theme.terminalAnsiBrightMagenta
    | 14 => theme.terminalAnsiBrightCyan
    | 15 => theme.terminalAnsiBrightWhite
    // For 256 colors, fall back to defaults
    | idx => ReveryTerminal.Theme.default(idx);

  let defaultBackground = theme.terminalBackground;
  let defaultForeground = theme.terminalForeground;

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
