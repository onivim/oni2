open EditorCoreTypes;

module Animation = {
  [@deriving show]
  type t =
    | Delete({
        line: EditorCoreTypes.LineNumber.t,
        count: int,
        animation: [@opaque] Component_Animation.t(float),
      })
    | Added({
        line: EditorCoreTypes.LineNumber.t,
        count: int,
        animation: [@opaque] Component_Animation.t(float),
      });

  let getOffset = (~rowHeight, ~line: LineNumber.t, animation) => {
    switch (animation) {
    | Delete({line: deletedLine, count, animation}) =>
      if (LineNumber.(line >= deletedLine)) {
        float(count) *. rowHeight *. Component_Animation.get(animation);
      } else {
        0.;
      }
    | Added({line: addedLine, count, animation}) =>
      if (LineNumber.(line >= addedLine + count)) {
        float(count) *. rowHeight *. Component_Animation.get(animation);
      } else {
        0.;
      }
    };
  };

  let update = (msg, animation) => {
    switch (animation) {
    | Delete({line, count, animation}) =>
      let animation' = Component_Animation.update(msg, animation);

      if (Component_Animation.isComplete(animation')) {
        None;
      } else {
        Some(Delete({line, count, animation: animation'}));
      };
    | Added({line, count, animation}) =>
      let animation' = Component_Animation.update(msg, animation);

      if (Component_Animation.isComplete(animation')) {
        None;
      } else {
        Some(Added({line, count, animation: animation'}));
      };
    };
  };

  let ofMinimalUpdate = (update: Oni_Core.MinimalUpdate.update) => {
    switch (update) {
    | Deleted({startLine, stopLine}) =>
      let animation: Component_Animation.t(float) =
        Component_Animation.make(
          Revery.UI.Animation.(
            animate(Revery.Time.milliseconds(75))
            |> ease(Revery.UI.Easing.cubic)
            |> tween(0.4, 0.0)
          ),
        );
      Some(
        Delete({
          line: startLine,
          count:
            LineNumber.toZeroBased(stopLine)
            - LineNumber.toZeroBased(startLine),
          animation,
        }),
      );
    | Added({beforeLine, lines}) =>
      let animation: Component_Animation.t(float) =
        Component_Animation.make(
          Revery.UI.Animation.(
            animate(Revery.Time.milliseconds(75))
            |> ease(Revery.UI.Easing.cubic)
            |> tween(-0.4, 0.)
          ),
        );
      Some(
        Added({line: beforeLine, count: Array.length(lines), animation}),
      );
    | _ => None
    };
  };
};

[@deriving show]
type t = {animations: list(Animation.t)};

let initial = {animations: []};

let getYOffset = (~rowHeight, ~line: LineNumber.t, model) => {
  let ret =
    if (model.animations == []) {
      0.;
    } else {
      model.animations
      |> List.fold_left(
           (acc, curr) => {
             let offset = Animation.getOffset(~rowHeight, ~line, curr);
             acc +. offset;
           },
           0.,
         );
    };
  ret;
};

let applyMinimalUpdate = (minimalUpdate, model) => {
  let newAnimations =
    minimalUpdate |> List.filter_map(Animation.ofMinimalUpdate);

  {animations: newAnimations @ model.animations};
};

let update = (msg, model) => {
  {animations: model.animations |> List.filter_map(Animation.update(msg))};
};

let isAnimating = model => model.animations != [];
