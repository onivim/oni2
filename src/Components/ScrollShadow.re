open Revery.UI;

module Gradient = {
  let paint = Skia.Paint.make();

  let drawLeftToRight =
      (
        ~opacity,
        ~leftColor: Revery.Color.t,
        ~rightColor: Revery.Color.t,
        ~x,
        ~y,
        ~width,
        ~height,
        ~context,
      ) => {
    let left = x;
    let top = y;
    let right = x +. width;

    let gradient =
      Skia.Shader.makeLinearGradient2(
        ~startPoint=Skia.Point.make(left, 0.0),
        ~stopPoint=Skia.Point.make(right, 0.0),
        ~startColor=leftColor |> Revery.Color.toSkia,
        ~stopColor=rightColor |> Revery.Color.toSkia,
        ~tileMode=`repeat,
      );

    Skia.Paint.setShader(paint, gradient);
    Skia.Paint.setAlpha(paint, opacity);

    Skia.Canvas.drawRectLtwh(context, left, top, width, height, paint);
  };

  let drawTopToBottom =
      (
        ~opacity,
        ~topColor: Revery.Color.t,
        ~bottomColor: Revery.Color.t,
        ~x,
        ~y,
        ~width,
        ~height,
        ~context,
      ) => {
    let left = x;
    let top = y;
    let bottom = y +. height;

    let gradient =
      Skia.Shader.makeLinearGradient2(
        ~startPoint=Skia.Point.make(0.0, top),
        ~stopPoint=Skia.Point.make(0.0, bottom),
        ~startColor=topColor |> Revery.Color.toSkia,
        ~stopColor=bottomColor |> Revery.Color.toSkia,
        ~tileMode=`repeat,
      );

    Skia.Paint.setShader(paint, gradient);
    Skia.Paint.setAlpha(paint, opacity);

    Skia.Canvas.drawRectLtwh(context, left, top, width, height, paint);
  };
};

module Shadow = {
  let shadowStartColor = Revery.Color.rgba(0., 0., 0., 0.22);
  let shadowStopColor = Revery.Color.rgba(0., 0., 0., 0.);

  type direction =
    | Left
    | Right
    | Down
    | Up;

  let render =
      (
        ~color=shadowStartColor,
        ~opacity,
        ~direction,
        ~x,
        ~y,
        ~width,
        ~height,
        context,
      ) => {
    let opacity = Revery.Color.getAlpha(color) *. opacity;
    switch (direction) {
    | Right =>
      Gradient.drawLeftToRight(
        ~opacity,
        ~leftColor=color,
        ~rightColor=shadowStopColor,
        ~x,
        ~y,
        ~width,
        ~height,
        ~context,
      )
    | Left =>
      Gradient.drawLeftToRight(
        ~opacity,
        ~leftColor=shadowStopColor,
        ~rightColor=color,
        ~x,
        ~y,
        ~width,
        ~height,
        ~context,
      )
    | Down =>
      Gradient.drawTopToBottom(
        ~opacity,
        ~topColor=color,
        ~bottomColor=shadowStopColor,
        ~x,
        ~y,
        ~width,
        ~height,
        ~context,
      )
    | Up =>
      Gradient.drawTopToBottom(
        ~opacity,
        ~topColor=shadowStopColor,
        ~bottomColor=color,
        ~x,
        ~y,
        ~width,
        ~height,
        ~context,
      )
    };
  };
};

module Top = {
  let make = (~theme, ~opacity=1.0, ~height as h=12, ()) => {
    let destColor = Feature_Theme.Colors.shadow.from(theme);
    <Canvas
      style=Style.[
        position(`Absolute),
        top(0),
        left(0),
        right(0),
        height(h),
        pointerEvents(`Ignore),
      ]
      render={(canvasContext, dimensions) => {
        Shadow.render(
          ~opacity,
          ~color=destColor,
          ~direction=Shadow.Down,
          ~x=0.,
          ~y=0.,
          ~width=float(dimensions.width),
          ~height=float(dimensions.height),
          canvasContext.canvas,
        )
      }}
    />;
  };
};

module Bottom = {
  let make = (~theme, ~opacity=1.0, ~height as h=12, ()) => {
    let destColor = Feature_Theme.Colors.shadow.from(theme);
    <Canvas
      style=Style.[
        position(`Absolute),
        bottom(0),
        left(0),
        right(0),
        height(h),
        pointerEvents(`Ignore),
      ]
      render={(canvasContext, dimensions) => {
        Shadow.render(
          ~opacity,
          ~color=destColor,
          ~direction=Shadow.Up,
          ~x=0.,
          ~y=0.,
          ~width=float(dimensions.width),
          ~height=float(dimensions.height),
          canvasContext.canvas,
        )
      }}
    />;
  };
};
