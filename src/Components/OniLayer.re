/*
 * Label.re
 *
 * Container to help render Exthost.Label.t
 */

open Revery.UI;

let make =
    (
      ~key,
      ~style,
      ~condition,
      ~backgroundColor,
      ~config,
      ~onMouseDown=?,
      ~onMouseUp=?,
      ~onMouseMove=?,
      ~onMouseEnter=?,
      ~onMouseLeave=?,
      ~onMouseWheel=?,
      ~onMouseOver=?,
      ~onBoundingBoxChanged=?,
      ~children,
      (),
    ) =>
  if (Feature_Configuration.GlobalConfiguration.layers.get(config)) {
    <Layer
      key
      style
      condition
      backgroundColor
      ?onMouseDown
      ?onMouseUp
      ?onMouseMove
      ?onMouseEnter
      ?onMouseOver
      ?onMouseLeave
      ?onMouseWheel
      ?onBoundingBoxChanged>
      children
    </Layer>;
  } else {
    <View
      style
      ?onMouseOver
      ?onBoundingBoxChanged
      ?onMouseDown
      ?onMouseUp
      ?onMouseMove
      ?onMouseEnter
      ?onMouseLeave
      ?onMouseWheel>
      children
    </View>;
  };
