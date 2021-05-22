/*
 * TerminalView.re
 *
 * Component for the 'terminal' buffer renderer
 */

module TerminalColors = Colors;
open Revery;
open Revery.UI;

module Colors = TerminalColors;
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

module Terminal = {
  let%component make =
                (
                  ~isActive,
                  ~config,
                  ~terminal: Model.terminal,
                  ~font: Service_Font.font,
                  ~theme: Oni_Core.ColorTheme.Colors.t,
                  ~dispatch,
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

    let lineHeight =
      Configuration.lineHeight.get(config)
      |> Oni_Core.Utility.OptionEx.value_or_lazy(() => {
           Feature_Configuration.GlobalConfiguration.Editor.lineHeight.get(
             config,
           )
         });

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
               dispatch(Model.Resized({id: terminal.id, rows, columns}))
             });

          None;
        },
      );

    let Service_Font.{fontSize, smoothing, _} = font;

    let lineHeightSize =
      Oni_Core.LineHeight.calculate(~measuredFontHeight=fontSize, lineHeight);
    let terminalFont =
      EditorTerminal.Font.make(
        ~smoothing,
        ~size=fontSize,
        ~lineHeight=lineHeightSize,
        resolvedFont,
      );
    let onDimensionsChanged =
        (
          {height, width, _}: Revery.UI.NodeEvents.DimensionsChangedEventParams.t,
        ) => {
      // If we have a loaded font, figure out how many columns and rows we can show
      let rows =
        float_of_int(height) /. terminalFont.lineHeight |> int_of_float;
      let columns =
        float_of_int(width) /. terminalFont.characterWidth |> int_of_float;

      lastDimensions := Some((rows, columns));
      dispatch(Model.Resized({id: terminal.id, rows, columns}));
    };

    let terminalTheme = TerminalColors.theme(theme);
    let defaultBackground = TerminalColors.defaultBackground(theme);
    let defaultForeground = TerminalColors.defaultForeground(theme);
    let {screen, cursor, _}: Model.terminal = terminal;

    let element = {
      EditorTerminal.render(
        ~opacity,
        ~defaultBackground,
        ~defaultForeground,
        ~theme=terminalTheme,
        ~cursor,
        ~font=terminalFont,
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
};

let make =
    (
      ~isActive,
      ~config,
      ~id: int,
      ~terminals: Model.t,
      ~font: Service_Font.font,
      ~theme: Oni_Core.ColorTheme.Colors.t,
      ~dispatch,
      (),
    ) => {
  terminals
  |> Model.getTerminalOpt(id)
  |> Option.map(terminal => {
       <Terminal config isActive theme font terminal dispatch />
     })
  |> Option.value(~default=Revery.UI.React.empty);
};
