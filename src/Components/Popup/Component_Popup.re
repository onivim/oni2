open EditorCoreTypes;
open Oni_Core;
module Spring = Component_Animation.Spring;

module UniqueId =
  UniqueId.Make({});

type model = {
  width: float,
  height: float,
  enterExitSpring: Spring.t,
  direction: float,
  isAnimated: bool,
  position: option(PixelPosition.t),
  uniqueId: UniqueId.t,
};

let create = (~width, ~height) => {
  width,
  height,
  enterExitSpring: Spring.make(0.),
  direction: 1.0,
  isAnimated: true,
  position: None,
  uniqueId: UniqueId.create(~friendlyName="Component_Popup"),
};

type msg =
  | Show
  | Hide
  | PositionChanged(PixelPosition.t)
  | Animation(Component_Animation.msg);

let isVisible = model => {
  let target = model.enterExitSpring |> Spring.getTarget;

  target >= 1.;
};

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

let sub = (~isVisible: bool, ~pixelPosition: option(PixelPosition.t), model) => {
  let uniqueIdStr = model.uniqueId |> UniqueId.toString;

  let visibleSubUniqueId =
    Printf.sprintf("Component_Popup.Visible:%s.%b", uniqueIdStr, isVisible);

  let visibleSub =
    SubEx.value(~uniqueId=visibleSubUniqueId, isVisible ? Show : Hide);

  let positionSub =
    pixelPosition
    |> Option.map((position: PixelPosition.t) => {
         let positionSubUniqueId =
           Printf.sprintf(
             "Component_Popup.Position:%s.%f.%f",
             uniqueIdStr,
             position.x,
             position.y,
           );
         SubEx.value(
           ~uniqueId=positionSubUniqueId,
           PositionChanged(position),
         );
       })
    |> Option.value(~default=Isolinear.Sub.none);

  let springSub =
    Component_Animation.Spring.sub(model.enterExitSpring)
    |> Isolinear.Sub.map(msg => Animation(msg));

  [springSub, visibleSub, positionSub] |> Isolinear.Sub.batch;
};

module View = {
  open Revery.UI;

  module Styles = {
    let getTransform = (~direction, transitionAmount) => {
      let angle =
        Revery.Math.Angle.Degrees(
          (-45.0) *. direction *. (1.0 -. transitionAmount),
        );

      let deltaY = (1.0 -. transitionAmount) *. 20.0 *. direction;
      let scale = 1.0 -. (1.0 -. transitionAmount) *. 0.1;
      [
        Revery.UI.Transform.Scale(scale),
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
