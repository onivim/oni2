open EditorCoreTypes;

open Oni_Core;
open Revery.UI;

let cursorSpringOptions =
  Spring.Options.create(~stiffness=310., ~damping=30., ());

let%component make =
              (
                ~scrollX,
                ~scrollY,
                ~metrics: EditorMetrics.t,
                ~editorFont: Service_Font.font,
                ~buffer,
                ~mode: Vim.Mode.t,
                ~isActiveSplit,
                ~cursorPosition: Location.t,
                ~colors: Colors.t,
                ~windowIsFocused,
                (),
              ) => {
  let%hook () = React.Hooks.effect(Always, () => None);
  let line = Index.toZeroBased(cursorPosition.line);
  let column = Index.toZeroBased(cursorPosition.column);
  let lineCount = Buffer.getNumberOfLines(buffer);

  let (originalX, originalY, characterWidth) =
    if (lineCount <= 0 || line >= lineCount || !isActiveSplit) {
      (
        // If we don't have a line, we're not rendering anything anyway...
        // ...but we still need to engange our hooks
        0.,
        0.,
        0,
      );
    } else {
      let bufferLine = Buffer.getLine(line, buffer);
      let (offset, characterWidth) =
        BufferViewTokenizer.getCharacterPositionAndWidth(bufferLine, column);

      let x = float(offset) *. editorFont.measuredWidth;
      let y = float(line) *. editorFont.measuredHeight +. 0.5;
      (x, y, characterWidth);
    };

  let durationFunc = (currentValue, targetValue) =>
    if (Float.abs(targetValue -. currentValue) < 2.
        *. editorFont.measuredHeight) {
      if (mode == Insert) {
        Revery.Time.milliseconds(100);
      } else {
        Revery.Time.zero;
      };
    } else {
      Revery.Time.milliseconds(100);
    };

  let%hook y =
    Hooks.transition(
      ~durationFunc,
      ~delay={mode == Insert ? Revery.Time.milliseconds(-50): Revery.Time.zero},
      ~duration=Revery.Time.milliseconds(100),
      ~easing=Easing.easeIn,
      originalY,
    );
  let%hook x =
    Hooks.transition(
      ~durationFunc,
      ~duration=Revery.Time.milliseconds(100),
      ~easing=Easing.easeIn,
      originalX,
    );

  // Set the opacity of the text in the cursor based on distance.
  // Other, there is a jarring effect where the destination character
  // shows up early in the cursor.
  let deltaY = y -. originalY;
  let deltaX = x -. originalX;
  let deltaDistSquared = deltaY *. deltaY +. deltaX *. deltaX;
  let maxDistSquared = 8. *. 8.;

  let textOpacity =
    (maxDistSquared -. deltaDistSquared) /. maxDistSquared |> max(0.);

  <Canvas
    style=Style.[
      position(`Absolute),
      top(0),
      left(0),
      right(0),
      bottom(0),
    ]
    render={canvasContext => {
      let context =
        Draw.createContext(
          ~canvasContext,
          ~width=metrics.pixelWidth,
          ~height=metrics.pixelHeight,
          ~scrollX,
          ~scrollY,
          ~lineHeight=editorFont.measuredHeight,
          ~editorFont,
        );
      let line = Index.toZeroBased(cursorPosition.line);
      let column = Index.toZeroBased(cursorPosition.column);
      let lineCount = Buffer.getNumberOfLines(buffer);

      if (lineCount <= 0 || line >= lineCount || !isActiveSplit) {
        ();
      } else {
        let height = context.lineHeight;
        let background = colors.cursorBackground;
        let foreground = colors.cursorForeground;

        if (!windowIsFocused) {
          let width = float(characterWidth) *. context.charWidth;
          Draw.rect(~context, ~x, ~y, ~width=1., ~height, ~color=foreground);
          Draw.rect(~context, ~x, ~y, ~width, ~height=1., ~color=foreground);
          Draw.rect(
            ~context,
            ~x,
            ~y=y +. height -. 1.,
            ~width,
            ~height=1.,
            ~color=foreground,
          );
          Draw.rect(
            ~context,
            ~x=x +. width -. 1.,
            ~y,
            ~width=1.,
            ~height,
            ~color=foreground,
          );
        } else if (mode == Insert) {
          let width = 2.;
          Draw.rect(~context, ~x, ~y, ~width, ~height, ~color=foreground);
        } else {
          let width = float(characterWidth) *. context.charWidth;
          Draw.rect(~context, ~x, ~y, ~width, ~height, ~color=foreground);
          let bufferLine = Buffer.getLine(line, buffer);

          switch (BufferLine.subExn(~index=column, ~length=1, bufferLine)) {
          | exception _
          | "" => ()
          | text
              when BufferViewTokenizer.isWhitespace(ZedBundled.get(text, 0)) =>
            ()
          | text =>
            Draw.shapedText(
              ~context,
              ~x=x -. 0.5,
              ~y=y -. context.fontMetrics.ascent -. 0.5,
              ~color=background |> Revery.Color.multiplyAlpha(textOpacity),
              text,
            )
          };
        };
      };
    }}
  />;
};
