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

let%component make =
              (
                ~metrics: EditorMetrics.t,
                ~terminal: Feature_Terminal.terminal,
                ~font: Service_Font.font,
                ~theme: Oni_Core.ColorTheme.Colors.t,
                (),
              ) => {
  let maybeFont = Revery.Font.load(font.fontFile) |> Stdlib.Result.to_option;

  let%hook lastDimensions = Hooks.ref(None);

  // When the terminal id changes, we need to make sure we're dispatching the resized
  // event, too. The ideal fix would be to have this component 'keyed' on the `terminal.id`
  // but since we don't have the concept of a key prop, this `If` handler will be triggered
  // when the `terminal.id` changes, so we have an opportunity to set the size for a new terminal.
  let%hook () =
    React.Hooks.effect(
      If((!=), terminal.id),
      () => {
        lastDimensions^
        |> Option.iter(((rows, columns)) => {
             GlobalContext.current().dispatch(
               Actions.Terminal(
                 Feature_Terminal.Resized({id: terminal.id, rows, columns}),
               ),
             )
           });

        None;
      },
    );

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

        lastDimensions := Some((rows, columns));
        GlobalContext.current().dispatch(
          Actions.Terminal(
            Feature_Terminal.Resized({id: terminal.id, rows, columns}),
          ),
        );
      },
      maybeFont,
    );
  };

  let terminalTheme = Feature_Terminal.theme(theme);
  let defaultBackground = Feature_Terminal.defaultBackground(theme);
  let defaultForeground = Feature_Terminal.defaultForeground(theme);

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
