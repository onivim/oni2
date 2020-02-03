open EditorCoreTypes;
open Revery;
open Revery.Draw;

open Oni_Core;

module FontIcon = Oni_Components.FontIcon;

let drawUnderline =
    (
      ~offset=0.,
      ~buffer,
      ~leftVisibleColumn,
      ~transform,
      ~scrollY,
      ~editorFont: EditorFont.t,
      ~color=Colors.black,
      r: Range.t,
    ) => {
  let halfOffset = offset /. 2.0;
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

  Shapes.drawRect(
    ~transform,
    ~x=float(startOffset) *. editorFont.measuredWidth -. halfOffset,
    ~y=
      editorFont.measuredHeight
      *. float(Index.toZeroBased(r.start.line))
      -. scrollY
      -. halfOffset
      +. (editorFont.measuredHeight -. 2.),
    ~height=1.,
    ~width=
      offset
      +. max(float(endOffset - startOffset), 1.0)
      *. editorFont.measuredWidth,
    ~color,
    (),
  );
};

let renderRange =
    (
      ~offset=0.,
      ~buffer,
      ~leftVisibleColumn,
      ~transform,
      ~scrollY,
      ~editorFont: EditorFont.t,
      ~color=Colors.black,
      r: Range.t,
    ) => {
  let halfOffset = offset /. 2.0;
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

    Shapes.drawRect(
      ~transform,
      ~x=float(startOffset) *. editorFont.measuredWidth -. halfOffset,
      ~y=
        editorFont.measuredHeight
        *. float(Index.toZeroBased(r.start.line))
        -. scrollY
        -. halfOffset,
      ~height=editorFont.measuredHeight +. offset,
      ~width=
        offset
        +. max(float(endOffset - startOffset), 1.0)
        *. editorFont.measuredWidth,
      ~color,
      (),
    );
  };
};

let renderToken =
    (
      ~editorFont: EditorFont.t,
      ~theme: Theme.t,
      ~scrollX: float,
      ~scrollY: float,
      ~transform,
      token: BufferViewTokenizer.t,
    ) => {
  let x =
    editorFont.measuredWidth
    *. float(Index.toZeroBased(token.startPosition))
    -. scrollX;
  let y = scrollY;

  let backgroundColor = token.backgroundColor;

  switch (token.tokenType) {
  | Text =>
    Revery.Draw.Text.drawString(
      ~window=Revery.UI.getActiveWindow(),
      ~transform,
      ~x,
      ~y,
      ~backgroundColor,
      ~color=token.color,
      ~fontFamily=editorFont.fontFile,
      ~fontSize=editorFont.fontSize,
      token.text,
    )

  | Tab =>
    Revery.Draw.Text.drawString(
      ~window=Revery.UI.getActiveWindow(),
      ~transform,
      ~x=x +. editorFont.measuredWidth /. 4.,
      ~y=y +. editorFont.measuredHeight /. 4.,
      ~backgroundColor,
      ~color=theme.editorWhitespaceForeground,
      ~fontFamily="FontAwesome5FreeSolid.otf",
      ~fontSize=10,
      FontIcon.codeToIcon(0xf30b),
    )

  | Whitespace =>
    let size = 2.;
    let xOffset = editorFont.measuredWidth /. 2. -. 1.;
    let yOffset = editorFont.measuredHeight /. 2. -. 1.;

    for (i in 0 to String.length(token.text) - 1) {
      let xPos = x +. editorFont.measuredWidth *. float(i);

      Shapes.drawRect(
        ~transform,
        ~x=xPos +. xOffset,
        ~y=y +. yOffset,
        ~width=size,
        ~height=size,
        ~color=theme.editorWhitespaceForeground,
        (),
      );
    };
  };
};

let drawRuler = (~transform, ~metrics: EditorMetrics.t, ~color, x) =>
  Shapes.drawRect(
    ~transform,
    ~x,
    ~y=0.0,
    ~height=float(metrics.pixelHeight),
    ~width=1.,
    ~color,
    (),
  );

let drawLineHighlight =
    (
      ~transform,
      ~metrics: EditorMetrics.t,
      ~scrollY,
      ~lineHeight,
      ~color,
      line,
    ) =>
  Shapes.drawRect(
    ~transform,
    ~x=0.,
    ~y=lineHeight *. float(Index.toZeroBased(line)) -. scrollY,
    ~height=lineHeight,
    ~width=float(metrics.pixelWidth),
    ~color,
    (),
  );
