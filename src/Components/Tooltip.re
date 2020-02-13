open Oni_Core;
open Revery;
open Revery.UI;

module Constants = {
  let delay = Time.ms(400);
};

// MODEL

type t = {
  text: string,
  x: float,
  y: float,
};

// TOOLTIP

module Tooltip = {
  module Styles = {
    open Style;

    let tooltip = (~theme: Theme.t, ~x, ~y) => [
      position(`Absolute),
      left(int_of_float(x)),
      top(int_of_float(y)),
      backgroundColor(theme.menuBackground),
    ];

    let tooltipText = (~theme: Theme.t, ~font: UiFont.t) => [
      fontFamily(font.fontFile),
      fontSize(font.fontSize),
      color(theme.menuForeground),
    ];
  };

  let make = (~text, ~x, ~y, ~theme, ~font, ()) =>
    <View style={Styles.tooltip(~theme, ~x, ~y)}>
      <Text style={Styles.tooltipText(~theme, ~font)} text />
    </View>;
};

// OVERLAY

module Overlay = {
  let internalSetTooltip = ref(_ => ());

  module Styles = {
    open Style;

    let overlay = [
      position(`Absolute),
      top(0),
      bottom(0),
      left(0),
      right(0),
      pointerEvents(`Allow),
      cursor(MouseCursors.arrow),
    ];
  };

  let%component make = (~theme, ~font, ()) => {
    let%hook (current, setCurrent) = Hooks.state(None);
    internalSetTooltip := setCurrent;

    switch (current) {
    | Some({text, x, y}) => <Tooltip text x y theme font />
    | None => React.empty
    };
  };

  let setTooltip = tooltip => internalSetTooltip^(_ => Some(tooltip));
};

// HOTSPOT

module Trigger = {
  let make = (~children, ~text, ~style=[], ()) => {
    let onMouseMove = _ => ();
    let onMouseOut = (evt: NodeEvents.mouseMoveEventParams) =>
      Overlay.setTooltip({text, x: evt.mouseX, y: evt.mouseY});

    <View style onMouseMove onMouseOut> children </View>;
  };
};

include Trigger;
