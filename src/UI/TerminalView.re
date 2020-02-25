/*
 * TerminalView.re
 *
 * Component for the 'terminal' buffer renderer
 */

open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Core;

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
      ~editorFont: EditorFont.t,
      (),
    ) => {
  let maybeFont =
    Revery.Font.load(editorFont.fontFile) |> Stdlib.Result.to_option;

  let onDimensionsChanged = (({height, width, _}: Revery.UI.NodeEvents.DimensionsChangedEventParams.t)) => {
    print_endline (Printf.sprintf("TERMINAL DIMENSIONS: %dx%d", width, height ));
  };

  let element =
    Option.map(
      font => {
        let {screen, cursor, _}: Feature_Terminal.terminal = terminal;
        let size = editorFont.fontSize;
        let font = ReveryTerminal.Font.make(~size, font);
        ReveryTerminal.render(~screen, ~cursor, ~font);
      },
      maybeFont,
    )
    |> Option.value(~default=React.empty);
  <View onDimensionsChanged
    style={Styles.container(metrics)}> element </View>;
};
