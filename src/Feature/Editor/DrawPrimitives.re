open EditorCoreTypes;
open Revery;
open Revery.Draw;

open Oni_Core;

let drawUnderline =
    (
      ~offset=0.,
      ~buffer,
      ~leftVisibleColumn,
      ~transform,
      ~gutterWidth,
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
    ~x=
      gutterWidth
      +. float(startOffset)
      *. editorFont.measuredWidth
      -. halfOffset,
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
      ~gutterWidth,
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
      ~x=
        gutterWidth
        +. float(startOffset)
        *. editorFont.measuredWidth
        -. halfOffset,
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
