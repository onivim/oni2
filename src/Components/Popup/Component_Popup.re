open EditorCoreTypes;
open Oni_Core;
module Spring = Component_Animation.Spring;

type model = {
  width: float,
  height: float,
  enterExitSpring: Spring.t,
  direction: float,
  isAnimated: bool,
  position: option(PixelPosition.t),
};

let create = (~width, ~height) => {
  width,
  height,
  enterExitSpring: Spring.make(0.),
  direction: 1.0,
  isAnimated: true,
  position: None,
};

type msg =
  | Show
  | Hide
  | PositionChanged(PixelPosition.t)
  | Animation(Component_Animation.msg);

let update = (msg, model) => {
  switch (msg) {
  | Show => {
      ...model,
      enterExitSpring:
        Spring.set(
          ~instant=!model.isAnimated,
          ~position=100.0,
          model.enterExitSpring,
        ),
    }
  | PositionChanged(position) => {...model, position: Some(position)}
  | Hide => {
      ...model,
      enterExitSpring:
        Spring.set(
          ~instant=!model.isAnimated,
          ~position=0.0,
          model.enterExitSpring,
        ),
    }
  | Animation(animationMsg) => {
      ...model,
      enterExitSpring: Spring.update(animationMsg, model.enterExitSpring),
    }
  };
};

let configurationChanged = (~config as _, model) => {
  // TODO: Set ui.animation
  model;
};

let sub = (~isVisible: bool, ~x: float, ~y: float, model) => {
  let visibleSubUniqueId =
    "Component_Popup.Visible" ++ string_of_bool(isVisible);

  let visibleSub =
    SubEx.value(~uniqueId=visibleSubUniqueId, isVisible ? Show : Hide);

  let position = PixelPosition.{x, y};
  let positionSubUniqueId =
    Printf.sprintf("Component_Popup.Position.%f.%f", x, y);

  let positionSub =
    SubEx.value(~uniqueId=positionSubUniqueId, PositionChanged(position));

  let springSub =
    if (Component_Animation.Spring.isActive(model.enterExitSpring)) {
      Component_Animation.Spring.sub(model.enterExitSpring)
      |> Isolinear.Sub.map(msg => Animation(msg));
    } else {
      Isolinear.Sub.none;
    };

  [visibleSub, positionSub, springSub] |> Isolinear.Sub.batch;
};

module View = {
  open Revery.UI;

  module Styles = {
    let getTransform = (~direction, transitionAmount) => {
      let angle =
        Revery.Math.Angle.Degrees(
          (-60.0) *. direction *. (1.0 -. transitionAmount),
        );

      let deltaY = (1.0 -. transitionAmount) *. 30.0 *. direction;
      [
        Revery.UI.Transform.RotateX(angle),
        Revery.UI.Transform.TranslateY(deltaY),
      ];
    };
    let container =
        (
          ~direction,
          pixelX,
          pixelY,
          pixelWidth,
          pixelHeight,
          transitionAmount,
        ) =>
      Style.[
        position(`Absolute),
        top(pixelY),
        left(pixelX),
        width(pixelWidth),
        height(pixelHeight),
        opacity(transitionAmount),
        transform(getTransform(~direction, transitionAmount)),
      ];
  };

  let make = (~model, ~inner, ()) => {
    switch (model.position) {
    | None => React.empty
    | Some(position) =>
      let transition =
        Component_Animation.Spring.get(model.enterExitSpring) /. 100.;

      <View
        style={Styles.container(
          ~direction=model.direction,
          int_of_float(position.x),
          int_of_float(position.y),
          int_of_float(model.width),
          int_of_float(model.height),
          transition,
        )}>
        {inner(~transition)}
      </View>;
    };
  };
};
