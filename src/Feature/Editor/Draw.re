open EditorCoreTypes;
open Revery.Draw;

open Oni_Core;

module FontAwesome = Oni_Components.FontAwesome;
module FontIcon = Oni_Components.FontIcon;

type context = {
  canvasContext: CanvasContext.t,
  width: int,
  height: int,
  scrollX: float,
  scrollY: float,
  lineHeight: float,
  fontFamily: Revery.Font.Family.t,
  fontSize: float,
  charWidth: float,
  charHeight: float,
  smoothing: Revery.Font.Smoothing.t,
};

let createContext =
    (
      ~canvasContext,
      ~width,
      ~height,
      ~scrollX,
      ~scrollY,
      ~lineHeight,
      ~editorFont: Service_Font.font,
    ) => {
  {
    canvasContext,
    width,
    height,
    scrollX,
    scrollY,
    lineHeight,
    fontFamily: editorFont.fontFamily,
    fontSize: editorFont.fontSize,
    charWidth: editorFont.measuredWidth,
    charHeight: editorFont.measuredHeight,
    smoothing: editorFont.smoothing,
  };
};

let renderImmediate = (~context, ~count, render) =>
  ImmediateList.render(
    ~scrollY=context.scrollY,
    ~rowHeight=context.lineHeight,
    ~height=float(context.height),
    ~count,
    ~render=(i, offsetY) => render(i, offsetY +. context.scrollY),
    (),
  );

let drawRect = {
  let paint = Skia.Paint.make();

  (~context, ~x, ~y, ~width, ~height, ~color) => {
    Skia.Paint.setColor(paint, Revery.Color.toSkia(color));

    CanvasContext.drawRectLtwh(
      ~left=x -. context.scrollX,
      ~top=y -. context.scrollY,
      ~width,
      ~height,
      ~paint,
      context.canvasContext,
    );
  };
};
let rect = drawRect;

let drawText = (~context, ~x, ~y, ~paint, text) =>
  CanvasContext.drawText(
    ~x=x -. context.scrollX,
    ~y=y -. context.scrollY,
    ~paint,
    ~text,
    context.canvasContext,
  );
let text = drawText;

let drawShapedText = {
  let paint = Skia.Paint.make();
  Skia.Paint.setTextEncoding(paint, GlyphId);

  Skia.Paint.setLcdRenderText(paint, true);

  (~context, ~x, ~y, ~color, ~bold, ~italic, text) => {
    let font =
      Service_Font.resolveWithFallback(
        ~italic,
        bold ? Revery.Font.Weight.Bold : Revery.Font.Weight.Normal,
        context.fontFamily,
      );
    let text = Revery.Font.(shape(font, text) |> ShapeResult.getGlyphString);

    Revery.Font.Smoothing.setPaint(~smoothing=context.smoothing, paint);
    Skia.Paint.setTextSize(paint, context.fontSize);
    Skia.Paint.setTypeface(paint, Revery.Font.getSkiaTypeface(font));
    Skia.Paint.setColor(paint, Revery.Color.toSkia(color));

    drawText(~context, ~x, ~y, ~paint, text);
  };
};
let shapedText = drawShapedText;

let drawUtf8Text = {
  let paint = Skia.Paint.make();
  Skia.Paint.setTextEncoding(paint, Utf8);

  Skia.Paint.setLcdRenderText(paint, true);

  (~context, ~x, ~y, ~color, ~bold, ~italic, text) => {
    let font =
      Service_Font.resolveWithFallback(
        ~italic,
        bold ? Revery.Font.Weight.Bold : Revery.Font.Weight.Normal,
        context.fontFamily,
      );
    Revery.Font.Smoothing.setPaint(~smoothing=context.smoothing, paint);
    Skia.Paint.setTextSize(paint, context.fontSize);
    Skia.Paint.setTypeface(paint, Revery.Font.getSkiaTypeface(font));
    Skia.Paint.setColor(paint, Revery.Color.toSkia(color));

    drawText(~context, ~x, ~y, ~paint, text);
  };
};
let utf8Text = drawUtf8Text;

let underline =
    (
      ~context,
      ~buffer,
      ~leftVisibleColumn,
      ~color=Revery.Colors.black,
      r: Range.t,
    ) => {
  let line = Index.toZeroBased(r.start.line);
  let start = Index.toZeroBased(r.start.column);
  let endC = Index.toZeroBased(r.stop.column);

  let text = Buffer.getLine(line, buffer);
  let (startOffset, _) =
    BufferViewTokenizer.getCharacterPositionAndWidth(
      ~viewOffset=leftVisibleColumn,
      text,
      start,
    );
  let (endOffset, _) =
    BufferViewTokenizer.getCharacterPositionAndWidth(
      ~viewOffset=leftVisibleColumn,
      text,
      endC,
    );

  drawRect(
    ~context,
    ~x=float(startOffset) *. context.charWidth,
    ~y=
      context.charHeight
      *. float(Index.toZeroBased(r.start.line))
      +. (context.charHeight -. 2.),
    ~height=1.,
    ~width=max(float(endOffset - startOffset), 1.0) *. context.charWidth,
    ~color,
  );
};

let range =
    (
      ~context,
      ~padding=0.,
      ~buffer,
      ~leftVisibleColumn,
      ~color=Revery.Colors.black,
      r: Range.t,
    ) => {
  let doublePadding = padding *. 2.;
  let line = Index.toZeroBased(r.start.line);
  let start = Index.toZeroBased(r.start.column);
  let endC = Index.toZeroBased(r.stop.column);

  let lines = Buffer.getNumberOfLines(buffer);
  if (line < lines) {
    let text = Buffer.getLine(line, buffer);
    let (startOffset, _) =
      BufferViewTokenizer.getCharacterPositionAndWidth(
        ~viewOffset=leftVisibleColumn,
        text,
        start,
      );
    let (endOffset, _) =
      BufferViewTokenizer.getCharacterPositionAndWidth(
        ~viewOffset=leftVisibleColumn,
        text,
        endC,
      );
    let length = max(float(endOffset - startOffset), 1.0);

    drawRect(
      ~context,
      ~x=float(startOffset) *. context.charWidth -. padding,
      ~y=
        context.charHeight
        *. float(Index.toZeroBased(r.start.line))
        -. padding,
      ~height=context.charHeight +. doublePadding,
      ~width=length *. context.charWidth +. doublePadding,
      ~color,
    );
  };
};

let token =
    (~context, ~offsetY, ~colors: Colors.t, token: BufferViewTokenizer.t) => {
  let font =
    Service_Font.resolveWithFallback(
      ~italic=token.italic,
      token.bold ? Revery.Font.Weight.Bold : Revery.Font.Weight.Normal,
      context.fontFamily,
    );
  let fontMetrics = Revery.Font.getMetrics(font, context.fontSize);
  let x = context.charWidth *. float(Index.toZeroBased(token.startPosition));
  let y = offsetY -. fontMetrics.ascent;

  switch (token.tokenType) {
  | Text =>
    drawShapedText(
      ~context,
      ~x,
      ~y,
      ~color=token.color,
      ~bold=token.bold,
      ~italic=token.italic,
      token.text,
    )

  | Tab =>
    CanvasContext.Deprecated.drawString(
      ~x=x +. context.charWidth /. 4. -. context.scrollX,
      ~y=y -. context.scrollY,
      ~color=colors.whitespaceForeground,
      ~fontFamily=
        Revery.Font.Family.toPath(
          ~italic=false,
          ~mono=false,
          Normal,
          FontAwesome.FontFamily.solid,
        ),
      ~fontSize=10.,
      ~text=FontIcon.codeToIcon(FontAwesome.longArrowAltRight),
      context.canvasContext,
    )

  | Whitespace =>
    let size = 2.;
    let xOffset = context.charWidth /. 2. -. 1.;
    let yOffset = context.charHeight /. 2. -. 1.;

    for (i in 0 to String.length(token.text) - 1) {
      let xPos = x +. context.charWidth *. float(i);

      drawRect(
        ~context,
        ~x=xPos +. xOffset,
        ~y=y -. yOffset,
        ~width=size,
        ~height=size,
        ~color=colors.whitespaceForeground,
      );
    };
  };
};

let ruler = (~context, ~color, x) =>
  drawRect(
    ~context,
    ~x,
    ~y=context.scrollY,
    ~height=float(context.height),
    ~width=1.,
    ~color,
  );

let lineHighlight = (~context, ~color, line) =>
  drawRect(
    ~context,
    ~x=0.,
    ~y=context.lineHeight *. float(Index.toZeroBased(line)),
    ~height=context.lineHeight,
    ~width=float(context.width),
    ~color,
  );

module Gradient = {
  let paint = Skia.Paint.make();

  let drawLeftToRight =
      (
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

    CanvasContext.drawRectLtwh(
      ~left,
      ~top,
      ~width,
      ~height,
      ~paint,
      context.canvasContext,
    );
  };

  let drawTopToBottom =
      (
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

    CanvasContext.drawRectLtwh(
      ~left,
      ~top,
      ~width,
      ~height,
      ~paint,
      context.canvasContext,
    );
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

  let render = (~direction, ~x, ~y, ~width, ~height, ~context) => {
    switch (direction) {
    | Right =>
      Gradient.drawLeftToRight(
        ~leftColor=shadowStartColor,
        ~rightColor=shadowStopColor,
        ~x,
        ~y,
        ~width,
        ~height,
        ~context,
      )
    | Left =>
      Gradient.drawLeftToRight(
        ~leftColor=shadowStopColor,
        ~rightColor=shadowStartColor,
        ~x,
        ~y,
        ~width,
        ~height,
        ~context,
      )
    | Down =>
      Gradient.drawTopToBottom(
        ~topColor=shadowStartColor,
        ~bottomColor=shadowStopColor,
        ~x,
        ~y,
        ~width,
        ~height,
        ~context,
      )
    | Up =>
      Gradient.drawTopToBottom(
        ~topColor=shadowStopColor,
        ~bottomColor=shadowStartColor,
        ~x,
        ~y,
        ~width,
        ~height,
        ~context,
      )
    };
  };
};
