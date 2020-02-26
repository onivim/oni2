/*
 * TerminalView.re
 *
 * Component for the 'terminal' buffer renderer
 */

open Revery.UI;
open Oni_Model;

module EditorMetrics = Feature_Editor.EditorMetrics;

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
      ~editorFont: Service_Font.t,
      (),
    ) => {
  let maybeFont =
    Revery.Font.load(editorFont.fontFile) |> Stdlib.Result.to_option;

  let size = editorFont.fontSize;

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

  let element =
    Option.map(
      font => {
        let {screen, cursor, _}: Feature_Terminal.terminal = terminal;
        let font =
          ReveryTerminal.Font.make(
            ~smoothing=Revery.Font.Smoothing.default,
            ~size,
            font,
          );
        ReveryTerminal.render(~cursor, ~font, screen);
      },
      maybeFont,
    )
    |> Option.value(~default=React.empty);
  <View onDimensionsChanged style={Styles.container(metrics)}>
    element
  </View>;
};
