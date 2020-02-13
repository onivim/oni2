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

let drawRect = (~context, ~x, ~y, ~width, ~height, ~paint) =>
  CanvasContext.drawRectLtwh(
    ~left=x -. context.scrollX,
    ~top=y -. context.scrollY,
    ~width,
    ~height,
    ~paint,
    context.canvasContext,
  );
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

let underlinePaint = Skia.Paint.make();
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

  Skia.Paint.setColor(underlinePaint, Revery.Color.toSkia(color));

  drawRect(
    ~context,
    ~x=float(startOffset) *. context.charWidth,
    ~y=
      context.charHeight
      *. float(Index.toZeroBased(r.start.line))
      +. (context.charHeight -. 2.),
    ~height=1.,
    ~width=max(float(endOffset - startOffset), 1.0) *. context.charWidth,
    ~paint=underlinePaint,
  );
};

let rangePaint = Skia.Paint.make();
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

    Skia.Paint.setColor(rangePaint, Color.toSkia(color));

    drawRect(
      ~context,
      ~x=float(startOffset) *. context.charWidth -. padding,
      ~y=
        context.charHeight
        *. float(Index.toZeroBased(r.start.line))
        -. padding,
      ~height=context.charHeight +. doublePadding,
      ~width=length *. context.charWidth +. doublePadding,
      ~paint=rangePaint,
    );
  };
};

let textPaint = Skia.Paint.make();
Skia.Paint.setTextEncoding(textPaint, GlyphId);
Skia.Paint.setAntiAlias(textPaint, true);
Skia.Paint.setLcdRenderText(textPaint, true);
let whitespacePaint = Skia.Paint.make();
let token =
    (~context, ~offsetY, ~theme: Theme.t, token: BufferViewTokenizer.t) => {
  let x = context.charWidth *. float(Index.toZeroBased(token.startPosition));
  let y = offsetY -. context.fontMetrics.ascent;

  switch (token.tokenType) {
  | Text =>
    let paint = textPaint;
    Skia.Paint.setTextSize(paint, context.fontSize);
    Skia.Paint.setTypeface(paint, Revery.Font.getSkiaTypeface(context.font));
    Skia.Paint.setColor(paint, Color.toSkia(token.color));

    let text =
      Revery.Font.shape(context.font, token.text)
      |> Revery.Font.ShapeResult.getGlyphString;

    drawText(~context, ~x, ~y, ~paint=textPaint, text);

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

    Skia.Paint.setColor(
      whitespacePaint,
      Color.toSkia(theme.editorWhitespaceForeground),
    );

    for (i in 0 to String.length(token.text) - 1) {
      let xPos = x +. context.charWidth *. float(i);

      drawRect(
        ~context,
        ~x=xPos +. xOffset,
        ~y=y -. yOffset,
        ~width=size,
        ~height=size,
        ~paint=whitespacePaint,
      );
    };
  };
};

let rulerPaint = Skia.Paint.make();
let ruler = (~context, ~color, x) => {
  Skia.Paint.setColor(rulerPaint, Color.toSkia(color));

  drawRect(
    ~context,
    ~x,
    ~y=0.0,
    ~height=float(context.height),
    ~width=1.,
    ~paint=rulerPaint,
  );
};

let lineHighlightPaint = Skia.Paint.make();
let lineHighlight = (~context, ~color, line) => {
  Skia.Paint.setColor(lineHighlightPaint, Color.toSkia(color));

  drawRect(
    ~context,
    ~x=0.,
    ~y=context.lineHeight *. float(Index.toZeroBased(line)),
    ~height=context.lineHeight,
    ~width=float(context.width),
    ~paint=lineHighlightPaint,
  );
};
