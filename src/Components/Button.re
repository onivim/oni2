open Oni_Core;
open Revery.UI;

module LocalState = {
  type t = {isHovered: bool};

  let initial = {isHovered: false};

  type action =
    | MouseOver
    | MouseOut;

  let reduce = (msg, _model) => {
    switch (msg) {
    | MouseOver => {isHovered: true}
    | MouseOut => {isHovered: false}
    };
  };
};

module Animations = {
  let backgroundTransitionDuration = Revery.Time.milliseconds(250);
  let liftTransitionDuration = Revery.Time.milliseconds(100);
};

module Colors = Feature_Theme.Colors;
module Sneakable = Feature_Sneak.View.Sneakable;

module Styles = {
  open Style;

  let container = (~background, ~yOffset) => [
    Style.backgroundColor(background),
    padding(8),
    margin(8),
    transform([Transform.TranslateY(yOffset)]),
  ];

  let text = (~color) => [Style.color(color)];
};

let%component make =
              (
                ~label: string,
                ~theme: ColorTheme.Colors.t,
                ~font: UiFont.t,
                ~onClick: unit => unit,
                (),
              ) => {
  let%hook ({isHovered}: LocalState.t, dispatch) =
    Hooks.reducer(~initialState=LocalState.initial, LocalState.reduce);

  let background =
    isHovered
      ? Colors.Button.hoverBackground.from(theme)
      : Colors.Button.background.from(theme);

  let%hook background =
    CustomHooks.colorTransition(
      ~duration=Animations.backgroundTransitionDuration,
      background,
    );

  let%hook yOffset =
    Hooks.transition(
      ~duration=Animations.liftTransitionDuration,
      isHovered ? (-2.0) : 0.0,
    );

  let foreground = Colors.Button.foreground.from(theme);

  <Sneakable
    sneakId=label
    onClick
    onMouseEnter={_ => {dispatch(LocalState.MouseOver)}}
    onMouseLeave={_ => {dispatch(LocalState.MouseOut)}}
    style={Styles.container(~background, ~yOffset)}>
    <Text
      fontFamily={font.family}
      fontSize=12.
      text=label
      style={Styles.text(~color=foreground)}
    />
  </Sneakable>;
};
