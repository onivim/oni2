open Oni_Core;
open Revery;
open Revery.UI;

module Colors = Feature_Theme.Colors.Menu;

module Constants = {
  let delay = Time.ms(400);
  let offsetX = 10;
  let offsetY = 10;
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

    let tooltip = (~theme, ~x, ~y) => [
      position(`Absolute),
      left(int_of_float(x) + Constants.offsetX),
      top(int_of_float(y) + Constants.offsetY),
      backgroundColor(Colors.background.from(theme)),
      paddingVertical(3),
      paddingHorizontal(8),
      boxShadow(
        ~xOffset=3.,
        ~yOffset=3.,
        ~blurRadius=5.,
        ~spreadRadius=0.,
        ~color=Color.rgba(0., 0., 0., 0.2),
      ),
    ];

    let tooltipText = (~theme) => [
      color(Colors.foreground.from(theme)),
      textWrap(TextWrapping.NoWrap),
    ];
  };

  let make = (~text, ~x, ~y, ~theme, ~font: UiFont.t, ()) =>
    <View style={Styles.tooltip(~theme, ~x, ~y)}>
      <Text
        style={Styles.tooltipText(~theme)}
        fontFamily={font.family}
        fontSize={font.size}
        text
      />
    </View>;
};

// OVERLAY

module Overlay: {
  let setTooltip: t => unit;
  let clearTooltip: unit => unit;

  let make:
    (
      ~key: React.Key.t=?,
      ~theme: ColorTheme.Colors.t,
      ~font: UiFont.t,
      unit
    ) =>
    element;
} = {
  let internalSetTooltip = ref(_ => ());
  let debouncedSetTooltip = {
    let clearTimeout = ref(() => ());

    maybeTooltip => {
      clearTimeout^();
      switch (maybeTooltip) {
      | Some(tooltip) =>
        clearTimeout :=
          Tick.timeout(
            () => internalSetTooltip^(_ => Some(tooltip)),
            Constants.delay,
          )
      | None => internalSetTooltip^(_ => None)
      };
    };
  };

  let setTooltip = tooltip => debouncedSetTooltip(Some(tooltip));
  let clearTooltip = () => debouncedSetTooltip(None);

  let%component make = (~theme, ~font, ()) => {
    let%hook (current, setCurrent) = Hooks.state(None);
    internalSetTooltip := setCurrent;

    switch (current) {
    | Some({text, x, y}) => <Tooltip text x y theme font />
    | None => React.empty
    };
  };
};

// HOTSPOT

module Trigger = {
  let make = (~children, ~text, ~style=[], ()) => {
    let onMouseOver = (evt: NodeEvents.mouseMoveEventParams) =>
      Overlay.setTooltip({text, x: evt.mouseX, y: evt.mouseY});
    let onMouseMove = onMouseOver;
    let onMouseOut = _ => Overlay.clearTooltip();

    <View style onMouseOver onMouseMove onMouseOut> children </View>;
  };
};

include Trigger;
