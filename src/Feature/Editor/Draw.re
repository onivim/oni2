open EditorCoreTypes;
open Revery;
open Revery.Draw;

open Oni_Core;

module FontIcon = Oni_Components.FontIcon;

type context = {
  transform: Reglm.Mat4.t,
  width: int,
  height: int,
  scrollX: float,
  scrollY: float,
  lineHeight: float,
  fontFamily: string,
  fontSize: int,
  charWidth: float,
  charHeight: float,
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

let rect = (~context, ~x, ~y, ~width, ~height, ~color) =>
  Shapes.drawRect(
    ~transform=context.transform,
    ~x=x -. context.scrollX,
    ~y=y -. context.scrollY,
    ~height,
    ~width,
    ~color,
    (),
  );

let text =
    (
      ~context,
      ~x,
      ~y,
      ~backgroundColor,
      ~color,
      ~fontFamily=context.fontFamily,
      ~fontSize=context.fontSize,
      text,
    ) =>
  Revery.Draw.Text.drawString(
    ~window=Revery.UI.getActiveWindow(),
    ~transform=context.transform,
    ~x=x -. context.scrollX,
    ~y=y -. context.scrollY,
    ~backgroundColor,
    ~color,
    ~fontFamily,
    ~fontSize,
    text,
  );

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

  rect(
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

    rect(
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
  let y = offsetY;

  let backgroundColor = token.backgroundColor;

  switch (token.tokenType) {
  | Text =>
    text(~context, ~x, ~y, ~backgroundColor, ~color=token.color, token.text)

  | Tab =>
    text(
      ~context,
      ~x=x +. context.charWidth /. 4.,
      ~y=y +. context.charHeight /. 4.,
      ~backgroundColor,
      ~color=theme.editorWhitespaceForeground,
      ~fontFamily="FontAwesome5FreeSolid.otf",
      ~fontSize=10,
      FontIcon.codeToIcon(0xf30b),
    )

  | Whitespace =>
    let size = 2.;
    let xOffset = context.charWidth /. 2. -. 1.;
    let yOffset = context.charHeight /. 2. -. 1.;

    for (i in 0 to String.length(token.text) - 1) {
      let xPos = x +. context.charWidth *. float(i);

      rect(
        ~context,
        ~x=xPos +. xOffset,
        ~y=y +. yOffset,
        ~width=size,
        ~height=size,
        ~color=theme.editorWhitespaceForeground,
      );
    };
  };
};

let ruler = (~context, ~color, x) =>
  rect(
    ~context,
    ~x,
    ~y=0.0,
    ~height=float(context.height),
    ~width=1.,
    ~color,
  );

let lineHighlight = (~context, ~color, line) =>
  rect(
    ~context,
    ~x=0.,
    ~y=context.lineHeight *. float(Index.toZeroBased(line)),
    ~height=context.lineHeight,
    ~width=float(context.width),
    ~color,
  );
