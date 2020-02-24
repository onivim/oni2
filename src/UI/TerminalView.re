/*
 * TerminalView.re
 *
 * Component for the 'terminal' buffer renderer
 */

open Revery.UI;
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
  <View style={Styles.container(metrics)}> element </View>;
};
