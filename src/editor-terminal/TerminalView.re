module Color = Revery.Color;
module Colors = Revery.Colors;
open Revery.Draw;
open Revery.UI;

open Accumulator;

module Styles = {
  open Style;
  let container = bg => [
    backgroundColor(bg),
    position(`Absolute),
    justifyContent(`Center),
    alignItems(`Center),
    bottom(0),
    top(0),
    left(0),
    right(0),
  ];

  let scrollBarContainer = (~opacity as opac) => [
    position(`Absolute),
    bottom(0),
    top(0),
    right(0),
    opacity(opac),
  ];
};

type terminalSize = {
  width: int,
  height: int,
};

let make =
    (
      ~defaultBackground=?,
      ~defaultForeground=?,
      ~opacity=1.0,
      ~theme=Theme.default,
      ~scrollY: float,
      ~screen: Screen.t,
      ~cursor: Cursor.t,
      ~font: Font.t,
      (),
    ) => {
  let totalRows = Screen.getTotalRows(screen);
  let screenRows = Screen.getVisibleRows(screen);
  let scrollBackRows = totalRows - screenRows;

  let bg =
    switch (defaultBackground) {
    | Some(v) => v
    | None => theme(0)
    };

  let getFgColor = cell =>
    Screen.getForegroundColor(
      ~defaultBackground?,
      ~defaultForeground?,
      ~theme,
      cell,
    )
    |> Revery.Color.toSkia;

  let getBgColor = cell =>
    Screen.getBackgroundColor(
      ~defaultBackground?,
      ~defaultForeground?,
      ~theme,
      cell,
    )
    |> Revery.Color.toSkia;

  let element =
    <Canvas
      style={Styles.container(bg)}
      render={(canvasContext, _) => {
        let {
          font,
          lineHeight,
          characterWidth,
          characterHeight,
          fontSize,
          smoothing,
        }: Font.t = font;
        let defaultBackgroundColor = bg |> Color.toSkia;

        let backgroundPaint = Skia.Paint.make();
        Skia.Paint.setAntiAlias(backgroundPaint, false);

        let textPaint = Skia.Paint.make();
        let typeFace = Revery.Font.getSkiaTypeface(font);
        Skia.Paint.setTypeface(textPaint, typeFace);
        Skia.Paint.setTextSize(textPaint, fontSize);
        Revery.Font.Smoothing.setPaint(~smoothing, textPaint);

        Skia.Paint.setLcdRenderText(textPaint, true);

        let lineSpacingOffset =
          max(0., (lineHeight -. characterHeight) /. 2.);

        let columns = Screen.getColumns(screen);
        let rows = Screen.getTotalRows(screen);

        let renderBackground = (row, yOffset) =>
          {let accumulator =
             ref(
               BackgroundColorAccumulator.create(
                 (startColumn, endColumn, color) =>
                 if (color !== defaultBackgroundColor) {
                   Skia.Paint.setColor(backgroundPaint, color);
                   Skia.Paint.setAlpha(backgroundPaint, opacity);
                   CanvasContext.drawRectLtwh(
                     ~paint=backgroundPaint,
                     ~left=float(startColumn) *. characterWidth,
                     ~top=yOffset +. lineSpacingOffset,
                     ~height=lineHeight,
                     ~width=float(endColumn - startColumn) *. characterWidth,
                     canvasContext,
                   );
                 }
               ),
             );
           for (column in 0 to columns - 1) {
             let cell = Screen.getCell(~row, ~column, screen);

             let bgColor = getBgColor(cell);
             let item = BackgroundColorAccumulator.{column, color: bgColor};
             accumulator := Accumulator.add(item, accumulator^);
           };
           Accumulator.flush(accumulator^)};

        let renderText = (row, yOffset) =>
          {Skia.Paint.setTextEncoding(textPaint, GlyphId);
           let accumulator =
             ref(
               TextAccumulator.create((startColumn, buffer, color) => {
                 Skia.Paint.setColor(textPaint, color);
                 Skia.Paint.setAlpha(textPaint, opacity);
                 let str = Buffer.contents(buffer);
                 let tokens =
                   str
                   |> Revery.Font.shape(font)
                   |> Revery.Font.ShapeResult.getGlyphStrings;
                 List.iter(
                   ((skiaFace, str)) => {
                     Skia.Paint.setTypeface(textPaint, skiaFace);

                     CanvasContext.drawText(
                       ~paint=textPaint,
                       ~x=float(startColumn) *. characterWidth,
                       ~y=yOffset +. characterHeight +. lineSpacingOffset,
                       ~text=str,
                       canvasContext,
                     );
                   },
                   tokens,
                 );
               }),
             );

           for (column in 0 to columns - 1) {
             let cell = Screen.getCell(~row, ~column, screen);

             let fgColor = getFgColor(cell);

             let item =
               TextAccumulator.{column, color: fgColor, uchar: cell.char};
             accumulator := Accumulator.add(item, accumulator^);
           };
           Accumulator.flush(accumulator^)};

        let perLineRenderer =
          ImmediateList.render(
            ~scrollY,
            ~rowHeight=lineHeight,
            ~height=lineHeight *. float(rows),
            ~count=rows,
          );

        perLineRenderer(~render=renderBackground, ());
        perLineRenderer(~render=renderText, ());

        // If the cursor is visible, let's paint it now
        if (cursor.visible) {
          let cursorColor =
            defaultForeground
            |> Option.value(~default=theme(15))
            |> Revery.Color.toSkia;

          let (yOffset, width, height) =
            switch (cursor.shape) {
            | BarLeft => (0., 2., lineHeight)
            | Underline => (
                lineHeight -. 2. -. lineSpacingOffset,
                characterWidth,
                2.,
              )
            | Unknown
            | Block => (0., characterWidth, lineHeight)
            };

          Skia.Paint.setColor(textPaint, cursorColor);
          Skia.Paint.setAlpha(textPaint, opacity);
          CanvasContext.drawRectLtwh(
            ~paint=textPaint,
            ~left=float(cursor.column) *. characterWidth,
            ~top=
              yOffset
              +. float(scrollBackRows + cursor.row)
              *. lineHeight
              -. scrollY,
            ~width,
            ~height,
            canvasContext,
          );
        };
      }}
    />;

  element;
};
