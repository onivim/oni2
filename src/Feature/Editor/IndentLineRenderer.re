/*
 * IndentLineRenderer.re
 *
 * Logic for rendering indent lines in the buffer view
 */
open Revery.Draw;

open EditorCoreTypes;
open Oni_Core;

let rec getIndentLevel =
        (
          ~reverse=false,
          indentationSettings,
          buffer,
          startLine,
          endLine,
          line,
          previousIndentLevel,
        ) => {
  let lineText = Buffer.getLine(line, buffer) |> BufferLine.raw;

  /*
   * If the line isn't empty, we should use that lines indent level.
   *
   * If the line is empty, we should find the next non-blank line.
   * Then if the previous and next indent is within 1 indent level of each
   * other, take the larger one. Else, take the smaller indent level.
   *
   * If we hit the top or bottom of the buffer, just set the next line indent
   * to 0.
   */

  if (!Utility.StringEx.isWhitespaceOnly(lineText)) {
    Indentation.getLevel(indentationSettings, lineText);
  } else {
    let newLine = reverse ? line - 1 : line + 1;
    let nextLineLevel =
      if (newLine >= endLine || newLine <= startLine) {
        0;
      } else {
        getIndentLevel(
          ~reverse,
          indentationSettings,
          buffer,
          startLine,
          endLine,
          newLine,
          previousIndentLevel,
        );
      };

    if (abs(nextLineLevel - previousIndentLevel) <= 1
        && previousIndentLevel != 0
        && nextLineLevel != 0) {
      max(nextLineLevel, previousIndentLevel);
    } else {
      min(nextLineLevel, previousIndentLevel);
    };
  };
};

let getActiveIndentLevelAndRange =
    (
      ~indentLevelCache,
      ~cursorLineIndentLevel,
      ~cursorLine,
      ~buffer,
      ~startLine,
      ~endLine,
    ) => {
  let bufferLineCount = Buffer.getNumberOfLines(buffer);
  if (cursorLine < bufferLineCount) {
    let topFinished = ref(false);
    let topLine = ref(cursorLine - 1);
    let bottomLine = ref(cursorLine + 1);
    let bottomFinished = ref(false);
    let previousIndentLevel = ref(cursorLineIndentLevel);

    while (topLine^ >= startLine && ! topFinished^) {
      let indentLevel =
        Hashtbl.find_opt(indentLevelCache, topLine^)
        |> Option.value(~default=0);

      if (indentLevel < cursorLineIndentLevel) {
        topFinished := true;
      } else {
        decr(topLine);
        previousIndentLevel := indentLevel;
      };
    };

    previousIndentLevel := cursorLineIndentLevel;

    while (bottomLine^ <= endLine && ! bottomFinished^) {
      let indentLevel =
        Hashtbl.find_opt(indentLevelCache, bottomLine^)
        |> Option.value(~default=0);

      if (indentLevel < cursorLineIndentLevel) {
        bottomFinished := true;
      } else {
        incr(bottomLine);
        previousIndentLevel := indentLevel;
      };
    };
    Some((topLine^, bottomLine^));
  } else {
    None;
  };
};

module GlobalMutable = {
  let inactivePaint = Skia.Paint.make();
  let activePaint = Skia.Paint.make();
  let cachedIndentLevel = Hashtbl.create(100);
};

let render =
    (
      ~context: Draw.context,
      ~buffer: Buffer.t,
      ~startLine,
      ~endLine,
      ~cursorPosition: CharacterPosition.t,
      ~colors: Colors.t,
      ~showActive: bool,
      indentationSettings: IndentationSettings.t,
    ) => {
  /* First, calculate indentation level for all relevant lines*/
  Hashtbl.clear(GlobalMutable.cachedIndentLevel);

  let startLine = EditorCoreTypes.LineNumber.toZeroBased(startLine);
  let endLine = EditorCoreTypes.LineNumber.toZeroBased(endLine) + 1;
  let bufferLineCount = Buffer.getNumberOfLines(buffer);
  let cursorLine =
    EditorCoreTypes.LineNumber.toZeroBased(cursorPosition.line);
  let startLine = max(0, startLine);
  let endLine = min(bufferLineCount, endLine);
  let previousIndentLevel = ref(0);
  for (line in startLine to endLine - 1) {
    let level =
      getIndentLevel(
        indentationSettings,
        buffer,
        startLine,
        endLine,
        line,
        previousIndentLevel^,
      );

    previousIndentLevel := level;

    Hashtbl.add(GlobalMutable.cachedIndentLevel, line, level);
  };

  let cursorLineIndentLevel =
    Hashtbl.find_opt(GlobalMutable.cachedIndentLevel, cursorLine)
    |> Option.value(~default=0);

  let maybeActiveIndentRange =
    if (showActive && cursorLineIndentLevel >= 1) {
      getActiveIndentLevelAndRange(
        ~indentLevelCache=GlobalMutable.cachedIndentLevel,
        ~cursorLineIndentLevel,
        ~cursorLine,
        ~buffer,
        ~startLine,
        ~endLine,
      );
    } else {
      None;
    };

  /* Next, render *all* indent guides */

  let indentationWidthInPixels =
    float(indentationSettings.tabSize) *. context.charWidth;

  let editor = context.editor;
  let bufferPositionToPixel = lineIdx => {
    let ({x: pixelX, y: pixelY}: PixelPosition.t, _) =
      Editor.bufferBytePositionToPixel(
        ~position=
          BytePosition.{
            line: EditorCoreTypes.LineNumber.ofZeroBased(lineIdx),
            byte: ByteIndex.zero,
          },
        editor,
      );

    let x = pixelX;
    let y = pixelY;
    (x, y);
  };

  Skia.Paint.setColor(
    GlobalMutable.inactivePaint,
    Revery.Color.toSkia(colors.indentGuideBackground),
  );
  Skia.Paint.setColor(
    GlobalMutable.activePaint,
    Revery.Color.toSkia(colors.indentGuideActiveBackground),
  );

  let lineHeight = Editor.lineHeightInPixels(editor);
  for (line in startLine to endLine - 1) {
    let level =
      Hashtbl.find_opt(GlobalMutable.cachedIndentLevel, line)
      |> Option.value(~default=0);

    let (x, y) = bufferPositionToPixel(line);

    for (i in 0 to level - 1) {
      let isActive =
        if (i == cursorLineIndentLevel - 1) {
          switch (maybeActiveIndentRange) {
          | Some((top, bottom)) => line >= top && line < bottom
          | None => false
          };
        } else {
          false;
        };

      let paint =
        isActive ? GlobalMutable.activePaint : GlobalMutable.inactivePaint;

      CanvasContext.drawRectLtwh(
        ~left=x +. indentationWidthInPixels *. float(i),
        ~top=y,
        ~width=1.,
        ~height=lineHeight,
        ~paint,
        context.canvasContext,
      );
    };

    previousIndentLevel := level;
  };
};
