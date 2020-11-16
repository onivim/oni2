open EditorCoreTypes;
module EditorColors = Colors;
open Revery.UI;

open Oni_Core;

module Log = (val Log.withNamespace("Oni2.UI.EditorSurface"));

module Styles = {
  open Style;

  let overlay = [
    pointerEvents(`Ignore),
    position(`Absolute),
    top(0),
    left(0),
    right(0),
    bottom(0),
  ];
};

let make = (~key, ~opacity, ~config, ~pixelRanges: list(PixelRange.t), ()) => {
  let ranges = pixelRanges;

  let bg = EditorConfiguration.yankHighlightColor.get(config);

  let elements =
    ranges
    |> List.map((range: PixelRange.t) => {
         <View
           style=Style.[
             position(`Absolute),
             top(range.start.y |> int_of_float),
             left(range.start.x |> int_of_float),
             width(range.stop.x -. range.start.x |> int_of_float),
             height(range.stop.y -. range.start.y |> int_of_float),
             backgroundColor(bg),
           ]
         />
       })
    |> React.listToElement;

  <View key style=Styles.overlay>
    <Opacity opacity> elements </Opacity>
  </View>;
};
