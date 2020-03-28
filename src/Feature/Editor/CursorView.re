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

  let (x, y, characterWidth) =
    if (lineCount <= 0 || line >= lineCount || !isActiveSplit) {
      (0., 0., 0);
    } else {
      let bufferLine = Buffer.getLine(line, buffer);
      let (offset, characterWidth) =
        BufferViewTokenizer.getCharacterPositionAndWidth(bufferLine, column);

      let x = float(offset) *. editorFont.measuredWidth;
      let y = float(line) *. editorFont.measuredHeight +. 0.5;
      (x, y, characterWidth);
    };

  let%hook (y, _setScrollYImmediately) =
    Hooks.spring(
      ~target=y,
      ~restThreshold=1.,
      ~enabled=true,
      cursorSpringOptions,
    );
  let%hook (x, _setScrollXImmediately) =
    Hooks.spring(
      ~target=x,
      ~restThreshold=1.,
      ~enabled=true,
      cursorSpringOptions,
    );

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
              ~color=background,
              text,
            )
          };
        };
      };
    }}
  />;
};
