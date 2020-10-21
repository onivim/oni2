/*
 * TerminalView.re
 *
 * Component for the 'terminal' buffer renderer
 */

open Revery;
open Revery.UI;
open Oni_Model;

module Colors = Feature_Terminal.Colors;
module Theme = Feature_Theme;

module Constants = {
  let scrollBarThickness = 6;
  let scrollTrackColor = Color.rgba(0.0, 0.0, 0.0, 0.4);
  let scrollThumbColor = Color.rgba(0.5, 0.5, 0.5, 0.4);
};

module Styles = {
  let container = opac =>
    Style.[position(`Relative), flexGrow(1), opacity(opac)];
};

let%component make =
              (
                ~isActive,
                ~config,
                ~terminal: Feature_Terminal.terminal,
                ~font: Service_Font.font,
                ~theme: Oni_Core.ColorTheme.Colors.t,
                (),
              ) => {
  let resolvedFont =
    Service_Font.resolveWithFallback(
      Revery.Font.Weight.Normal,
      font.fontFamily,
    );

  let opacity =
    isActive
      ? 1.0
      : Feature_Configuration.GlobalConfiguration.inactiveWindowOpacity.get(
          config,
        );

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

  let Service_Font.{fontSize, smoothing, _} = font;

  let onDimensionsChanged =
      (
        {height, width, _}: Revery.UI.NodeEvents.DimensionsChangedEventParams.t,
      ) => {
    // If we have a loaded font, figure out how many columns and rows we can show
    let terminalFont = ReveryTerminal.Font.make(~size=fontSize, resolvedFont);
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
  };

  let terminalTheme = Feature_Terminal.theme(theme);
  let defaultBackground = Feature_Terminal.defaultBackground(theme);
  let defaultForeground = Feature_Terminal.defaultForeground(theme);

  let element = {
    let {screen, cursor, _}: Feature_Terminal.terminal = terminal;
    let font =
      ReveryTerminal.Font.make(~smoothing, ~size=fontSize, resolvedFont);
    ReveryTerminal.render(
      ~opacity,
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
  };
  <View onDimensionsChanged style={Styles.container(opacity)}>
    element
  </View>;
};
