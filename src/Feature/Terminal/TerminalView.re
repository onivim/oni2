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
                  ~theme: Oni_Core.ColorTheme.Colors.t,
                  ~dispatch,
                  (),
                ) => {
    let opacity =
      isActive
        ? 1.0
        : Feature_Configuration.GlobalConfiguration.inactiveWindowOpacity.get(
            config,
          );

    let font = terminal.font;

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
          |> Option.iter(((rows, columns, _pixelHeight)) => {
               dispatch(Model.Resized({id: terminal.id, rows, columns}))
             });

          None;
        },
      );

    let terminalFont = font;

    let onDimensionsChanged =
        (
          {height, width, _}: Revery.UI.NodeEvents.DimensionsChangedEventParams.t,
        ) => {
      // If we have a loaded font, figure out how many columns and rows we can show
      let rows =
        float_of_int(height) /. terminalFont.lineHeight |> int_of_float;
      let columns =
        float_of_int(width) /. terminalFont.characterWidth |> int_of_float;

      lastDimensions := Some((rows, columns, height));
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
        ~scrollY=terminal.scrollY,
        screen,
      );
    };

    let onMouseWheel = ({deltaY, _}: NodeEvents.mouseWheelEventParams) => {
      dispatch(
        Terminal({
          id: terminal.id,
          msg: MouseWheelScrolled({deltaY: deltaY *. (-25.0)}),
        }),
      );
    };

    let pixelHeight =
      switch (lastDimensions^) {
      | None => Terminal.viewportHeight(terminal)
      | Some((_, _, pixelY)) => float(pixelY)
      };

    let scrollBar =
      <Oni_Components.Scrollbar
        config
        theme
        pixelHeight
        viewportHeight={Terminal.viewportHeight(terminal)}
        totalHeight={Terminal.totalHeight(terminal)}
        scrollY={terminal.scrollY}
        onScroll={scrollY =>
          dispatch(
            Terminal({
              id: terminal.id,
              msg: ScrollbarMoved({scrollY: scrollY}),
            }),
          )
        }
      />;

    let elems = [element, scrollBar] |> React.listToElement;
    <View onDimensionsChanged onMouseWheel style={Styles.container(opacity)}>
      elems
    </View>;
  };
};

let make =
    (
      ~isActive,
      ~config,
      ~id: int,
      ~terminals: Model.t,
      ~theme: Oni_Core.ColorTheme.Colors.t,
      ~dispatch,
      (),
    ) => {
  terminals
  |> Model.getTerminalOpt(id)
  |> Option.map(terminal => {
       <Terminal config isActive theme terminal dispatch />
     })
  |> Option.value(~default=Revery.UI.React.empty);
};
