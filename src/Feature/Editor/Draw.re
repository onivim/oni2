open EditorCoreTypes;
open Revery.Draw;

module FontAwesome = Oni_Components.FontAwesome;
module FontIcon = Oni_Components.FontIcon;

type context = {
  canvasContext: CanvasContext.t,
  width: int,
  height: int,
  editor: Editor.t,
  fontFamily: Revery.Font.Family.t,
  fontWeight: Revery.Font.Weight.t,
  fontSize: float,
  charWidth: float,
  charHeight: float,
  smoothing: Revery.Font.Smoothing.t,
  features: list(Revery.Font.Feature.t),
};

let createContext =
    (
      ~canvasContext,
      ~width,
      ~height,
      ~editor: Editor.t,
      ~editorFont: Service_Font.font,
    ) => {
  {
    canvasContext,
    width,
    height,
    editor,
    fontFamily: editorFont.fontFamily,
    fontWeight: editorFont.fontWeight,
    fontSize: editorFont.fontSize,
    charWidth: editorFont.spaceWidth,
    charHeight: editorFont.measuredHeight,
    smoothing: editorFont.smoothing,
    features: editorFont.features,
  };
};

let renderImmediate = (~context, render) => {
  let topLine = max(0, Editor.getTopViewLine(context.editor) - 1);
  let bottomLine =
    min(
      Editor.totalViewLines(context.editor) - 1,
      Editor.getBottomViewLine(context.editor) + 1,
    );

  for (idx in topLine to bottomLine) {
    let offsetY = Editor.viewLineToPixelY(idx, context.editor);

    render(idx, offsetY -. Editor.scrollY(context.editor));
  };
};

let drawRect = {
  let paint = Skia.Paint.make();

  (~context, ~x, ~y, ~width, ~height, ~color) => {
    Skia.Paint.setColor(paint, Revery.Color.toSkia(color));

    CanvasContext.drawRectLtwh(
      ~left=x,
      ~top=y,
      ~width,
      ~height,
      ~paint,
      context.canvasContext,
    );
  };
};

let drawSquiggly = {
  let paint = Skia.Paint.make();

  let squiggleHeight = 1.4;

  let squigglePath = Skia.Path.make();
  Skia.Path.lineTo(squigglePath, squiggleHeight, squiggleHeight);
  Skia.Path.lineTo(
    squigglePath,
    squiggleHeight *. 3.0,
    (-1.0) *. squiggleHeight,
  );
  Skia.Path.lineTo(squigglePath, squiggleHeight *. 4.0, 0.0);

  let scale = squiggleHeight *. 4.0;
  let translate =
    Skia.Matrix.makeScale(
      scale,
      squiggleHeight,
      squiggleHeight,
      squiggleHeight,
    );

  Skia.Paint.setStyle(paint, Stroke);
  Skia.Paint.setAntiAlias(paint, true);
  Skia.Paint.setStrokeWidth(paint, 1.);

  (~context, ~x, ~y, ~width, ~color) => {
    // Shift the squiggle left a bit, halfway through the start of the squiggle
    let left = x -. squiggleHeight *. 2.;
    let top = y -. squiggleHeight;
    let height = squiggleHeight;

    // Confusingly, the Skia 2D path effect (which we used to draw the squiggle)
    // starts the pattern with respect to the _screen_ top/left coordinates.
    // We need to offset this so it's correct for the diagnostic itself,
    // so we compute the remainder based on the y and squiggle height,
    // and transform to 'undo' the screen space shift.
    let translateY = Float.rem(top, squiggleHeight);
    Skia.Matrix.setTranslateY(translate, 1.0 *. translateY);

    let pathEffect =
      Skia.PathEffect.create2dPath(~matrix=translate, squigglePath);
    Skia.Paint.setPathEffect(paint, pathEffect);

    let color = color |> Revery.Color.toSkia;
    Skia.Paint.setColor(paint, color);
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

let rect = drawRect;

let drawText = (~context, ~x, ~y, ~paint, text) =>
  CanvasContext.drawText(~x, ~y, ~paint, ~text, context.canvasContext);
let text = drawText;

let drawShapedText = {
  let paint = Skia.Paint.make();
  Skia.Paint.setTextEncoding(paint, GlyphId);

  Skia.Paint.setLcdRenderText(paint, true);

  (~context, ~x, ~y, ~color, ~bold, ~italic, text) => {
    let font =
      Service_Font.resolveWithFallback(
        ~italic,
        bold ? Oni_Core.Font.bolder(context.fontWeight) : context.fontWeight,
        context.fontFamily,
      );
    let text =
      Revery.Font.(
        shape(~features=context.features, font, text)
        |> ShapeResult.getGlyphStrings
      );

    Revery.Font.Smoothing.setPaint(~smoothing=context.smoothing, paint);
    Skia.Paint.setTextSize(paint, context.fontSize);
    Skia.Paint.setColor(paint, Revery.Color.toSkia(color));

    let offset = ref(x);

    text
    |> List.iter(((skiaFace, str)) => {
         Skia.Paint.setTypeface(paint, skiaFace);

         drawText(~context, ~x=offset^, ~y, ~paint, str);

         offset := offset^ +. Skia.Paint.measureText(paint, str, None);
       });
  };
};

let shapedText = drawShapedText;

let drawUtf8Text = {
  let paint = Skia.Paint.make();
  Skia.Paint.setTextEncoding(paint, Utf8);

  Skia.Paint.setLcdRenderText(paint, true);

  (~context, ~x, ~y, ~color, ~bold, ~italic, ~text) => {
    let font =
      Service_Font.resolveWithFallback(
        ~italic,
        bold ? Oni_Core.Font.bolder(context.fontWeight) : context.fontWeight,
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
    (~context, ~color=Revery.Colors.black, range: CharacterRange.t) => {
  let ({y: startPixelY, x: startPixelX}: PixelPosition.t, _) =
    Editor.bufferCharacterPositionToPixel(
      ~position=range.start,
      context.editor,
    );

  let ({x: stopPixelX, _}: PixelPosition.t, _) =
    Editor.bufferCharacterPositionToPixel(
      ~position=range.stop,
      context.editor,
    );

  let paddingY = context.editor |> Editor.linePaddingInPixels;

  drawRect(
    ~context,
    ~x=startPixelX,
    ~y=startPixelY -. paddingY +. Editor.lineHeightInPixels(context.editor),
    ~height=1.,
    ~width=max(stopPixelX -. startPixelX, 1.0),
    ~color,
  );
};

let strikethrough =
    (~context, ~color=Revery.Colors.black, range: CharacterRange.t) => {
  let ({y: startPixelY, x: startPixelX}: PixelPosition.t, _) =
    Editor.bufferCharacterPositionToPixel(
      ~position=range.start,
      context.editor,
    );

  let ({x: stopPixelX, _}: PixelPosition.t, _) =
    Editor.bufferCharacterPositionToPixel(
      ~position=range.stop,
      context.editor,
    );

  drawRect(
    ~context,
    ~x=startPixelX -. 1.,
    ~y=startPixelY +. Editor.lineHeightInPixels(context.editor) /. 2.,
    ~height=1.,
    ~width=max(stopPixelX -. startPixelX, 1.0) +. 2.,
    ~color,
  );
};

let squiggly = (~context, ~color=Revery.Colors.black, range: CharacterRange.t) => {
  let ({x: startPixelX, y: startPixelY}: PixelPosition.t, _) =
    Editor.bufferCharacterPositionToPixel(
      ~position=range.start,
      context.editor,
    );

  let ({x: stopPixelX, _}: PixelPosition.t, _) =
    Editor.bufferCharacterPositionToPixel(
      ~position=range.stop,
      context.editor,
    );

  let paddingY = context.editor |> Editor.linePaddingInPixels;

  drawSquiggly(
    ~context,
    ~x=startPixelX,
    ~y=startPixelY -. paddingY +. Editor.lineHeightInPixels(context.editor),
    ~width=max(stopPixelX -. startPixelX, 1.0),
    ~color,
  );
};

let rangeCharacter =
    (~context, ~padding=0., ~color=Revery.Colors.black, r: CharacterRange.t) => {
  let doublePadding = padding *. 2.;
  //  let line = Index.toZeroBased(r.start.line);
  //  let start = Index.toZeroBased(r.start.column);
  //  let endC = Index.toZeroBased(r.stop.column);
  //  let endLine = Index.toZeroBased(r.stop.line);

  let ({y: startPixelY, x: startPixelX}: PixelPosition.t, _) =
    Editor.bufferCharacterPositionToPixel(~position=r.start, context.editor);

  let ({x: stopPixelX, _}: PixelPosition.t, _) =
    Editor.bufferCharacterPositionToPixel(~position=r.stop, context.editor);

  let lineHeight = Editor.lineHeightInPixels(context.editor);
  let characterWidth = Editor.characterWidthInPixels(context.editor);

  drawRect(
    ~context,
    ~x=startPixelX,
    ~y=startPixelY,
    ~height=lineHeight +. doublePadding,
    ~width=max(stopPixelX -. startPixelX, characterWidth),
    ~color,
  );
};

let rangeByte =
    (~context, ~padding=0., ~color=Revery.Colors.black, r: ByteRange.t) => {
  let doublePadding = padding *. 2.;

  let startViewLine =
    Editor.bufferBytePositionToViewLine(r.start, context.editor);
  let stopViewLine =
    Editor.bufferBytePositionToViewLine(r.stop, context.editor);

  let ({x: startPixelX, _}: PixelPosition.t, _) =
    Editor.bufferBytePositionToPixel(~position=r.start, context.editor);

  let ({x: stopPixelX, _}: PixelPosition.t, _) =
    Editor.bufferBytePositionToPixel(~position=r.stop, context.editor);

  let lineHeight = Editor.lineHeightInPixels(context.editor);
  let characterWidth = Editor.characterWidthInPixels(context.editor);

  for (idx in startViewLine to stopViewLine) {
    let y =
      Editor.viewLineToPixelY(idx, context.editor)
      -. Editor.scrollY(context.editor);
    let startX =
      if (idx == startViewLine) {
        startPixelX;
      } else {
        0.;
      };

    let stopX =
      if (idx == stopViewLine) {
        stopPixelX;
      } else {
        Editor.getTotalWidthInPixels(context.editor);
      };

    drawRect(
      ~context,
      ~x=startX,
      ~y,
      ~height=lineHeight +. doublePadding,
      ~width=max(stopX -. startX, characterWidth),
      ~color,
    );
  };
};

let tabPaint = Skia.Paint.make();
Skia.Paint.setTextEncoding(tabPaint, GlyphId);
Skia.Paint.setLcdRenderText(tabPaint, true);
Skia.Paint.setAntiAlias(tabPaint, true);
Skia.Paint.setTextSize(tabPaint, 10.);
Skia.Paint.setTextEncoding(tabPaint, Utf8);

let token =
    (~context, ~offsetY, ~colors: Colors.t, token: BufferViewTokenizer.t) => {
  let font =
    Service_Font.resolveWithFallback(
      ~italic=token.italic,
      token.bold
        ? Oni_Core.Font.bolder(context.fontWeight) : context.fontWeight,
      context.fontFamily,
    );

  let fontMetrics = Revery.Font.getMetrics(font, context.fontSize);
  let pixelY = offsetY;
  let pixelX = token.startPixel;
  let paddingY = context.editor |> Editor.linePaddingInPixels;
  let y = paddingY +. pixelY -. fontMetrics.ascent;
  let x = pixelX;

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
    Skia.Paint.setColor(
      tabPaint,
      Revery_Core.Color.toSkia(colors.whitespaceForeground),
    );
    FontAwesome.FontFamily.solid
    |> Revery.Font.Family.toSkia(Normal)
    |> Revery.Font.load
    |> Result.get_ok
    |> Revery.Font.getSkiaTypeface
    |> Skia.Paint.setTypeface(tabPaint);
    CanvasContext.drawText(
      ~paint=tabPaint,
      ~x=x +. context.charWidth /. 4.,
      ~y,
      ~text=FontIcon.codeToIcon(FontAwesome.longArrowAltRight),
      context.canvasContext,
    );

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
    ~y=0.,
    ~height=float(context.height),
    ~width=1.,
    ~color,
  );

let lineHighlight = (~context, ~color, viewLine) => {
  let pixelY = Editor.viewLineToPixelY(viewLine, context.editor);

  drawRect(
    ~context,
    ~x=0.,
    ~y=pixelY -. Editor.scrollY(context.editor),
    ~height=Editor.lineHeightInPixels(context.editor),
    ~width=float(context.width),
    ~color,
  );
};

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
  let shadowStopColor = Revery.Color.rgba(0., 0., 0., 0.);

  type direction =
    | Left
    | Right
    | Down
    | Up;

  let render = (~color, ~direction, ~x, ~y, ~width, ~height, ~context) => {
    switch (direction) {
    | Right =>
      Gradient.drawLeftToRight(
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
