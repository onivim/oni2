open EditorCoreTypes;
open Revery;
open Revery.Draw;

open Oni_Core;

module FontIcon = Oni_Components.FontIcon;

type context = {
  canvasContext: CanvasContext.t,
  width: int,
  height: int,
  scrollX: float,
  scrollY: float,
  lineHeight: float,
  font: Revery.Font.t,
  fontMetrics: Revery.Font.FontMetrics.t,
  fontSize: float,
  charWidth: float,
  charHeight: float,
};

let createContext =
    (
      ~canvasContext,
      ~width,
      ~height,
      ~scrollX,
      ~scrollY,
      ~lineHeight,
      ~editorFont: EditorFont.t,
    ) => {
  let font = Revery.Font.load(editorFont.fontFile) |> Stdlib.Result.get_ok;
  let fontMetrics = Revery.Font.getMetrics(font, editorFont.fontSize);

  {
    canvasContext,
    width,
    height,
    scrollX,
    scrollY,
    lineHeight,
    font,
    fontMetrics,
    fontSize: editorFont.fontSize,
    charWidth: editorFont.measuredWidth,
    charHeight: editorFont.measuredHeight,
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

  Revery.Font.Smoothing.setPaint(
    ~smoothing=Revery.Font.Smoothing.default,
    paint,
  );
  Skia.Paint.setLcdRenderText(paint, true);

  (~context, ~x, ~y, ~color, text) => {
    let text =
      Revery.Font.(shape(context.font, text) |> ShapeResult.getGlyphString);

    Skia.Paint.setTextSize(paint, context.fontSize);
    Skia.Paint.setTypeface(paint, Revery.Font.getSkiaTypeface(context.font));
    Skia.Paint.setColor(paint, Revery.Color.toSkia(color));

    drawText(~context, ~x, ~y, ~paint, text);
  };
};
let shapedText = drawShapedText;

let drawUtf8Text = {
  let paint = Skia.Paint.make();
  Skia.Paint.setTextEncoding(paint, Utf8);

  Revery.Font.Smoothing.setPaint(
    ~smoothing=Revery.Font.Smoothing.default,
    paint,
  );
  Skia.Paint.setLcdRenderText(paint, true);

  (~context, ~x, ~y, ~color, text) => {
    Skia.Paint.setTextSize(paint, context.fontSize);
    Skia.Paint.setTypeface(paint, Revery.Font.getSkiaTypeface(context.font));
    Skia.Paint.setColor(paint, Revery.Color.toSkia(color));

    drawText(~context, ~x, ~y, ~paint, text);
  };
};
let utf8Text = drawUtf8Text;

let underline =
    (~context, ~buffer, ~leftVisibleColumn, ~color=Colors.black, r: Range.t) => {
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
      ~color=Colors.black,
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
    (~context, ~offsetY, ~theme: Theme.t, token: BufferViewTokenizer.t) => {
  let x = context.charWidth *. float(Index.toZeroBased(token.startPosition));
  let y = offsetY -. context.fontMetrics.ascent;

  switch (token.tokenType) {
  | Text => drawShapedText(~context, ~x, ~y, ~color=token.color, token.text)

  | Tab =>
    CanvasContext.Deprecated.drawString(
      ~x=x +. context.charWidth /. 4.,
      ~y,
      ~color=theme.editorWhitespaceForeground,
      ~fontFamily="FontAwesome5FreeSolid.otf",
      ~fontSize=10.,
      ~text=FontIcon.codeToIcon(0xf30b),
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
        ~color=theme.editorWhitespaceForeground,
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
