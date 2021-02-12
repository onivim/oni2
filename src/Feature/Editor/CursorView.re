open EditorCoreTypes;

open Oni_Core;
open Revery.UI;

module Config = EditorConfiguration;

let cursorSpringOptions =
  Spring.Options.create(~stiffness=310., ~damping=30., ());

let%component make =
              (
                ~editor: Editor.t,
                ~editorFont: Service_Font.font,
                ~mode: Vim.Mode.t,
                ~isActiveSplit,
                ~cursorPosition: CharacterPosition.t,
                ~colors: Colors.t,
                ~windowIsFocused,
                ~config,
                (),
              ) => {
  let%hook () = React.Hooks.effect(Always, () => None);

  let ({x: pixelX, y: pixelY}: PixelPosition.t, characterWidth) =
    Editor.bufferCharacterPositionToPixel(~position=cursorPosition, editor);

  let originalX = pixelX;
  let originalY = pixelY;

  let durationFunc = (~current, ~target) =>
    if (Float.abs(target -. current) < 2. *. editorFont.measuredHeight) {
      if (Vim.Mode.isInsert(mode)) {
        Revery.Time.milliseconds(50);
      } else {
        Revery.Time.zero;
      };
    } else {
      Revery.Time.milliseconds(100);
    };

  let animatedCursor =
    Config.Experimental.cursorSmoothCaretAnimation.get(config);

  let delay = Revery.Time.zero;
  let defaultDuration = Revery.Time.milliseconds(100);
  let easing = Easing.easeIn;

  let%hook y =
    Hooks.transitionf(
      ~name="Cursor Y Transition",
      ~active=animatedCursor,
      ~duration=durationFunc,
      ~delay,
      ~initialDuration=defaultDuration,
      ~easing,
      originalY,
    );
  let%hook x =
    Hooks.transitionf(
      ~name="Cursor X Transition",
      ~active=animatedCursor,
      ~delay,
      ~duration=durationFunc,
      ~initialDuration=defaultDuration,
      ~easing,
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

  let characterPaddingY = editor |> Editor.linePaddingInPixels;

  <Canvas
    style=Style.[
      position(`Absolute),
      top(0),
      left(0),
      right(0),
      bottom(0),
    ]
    render={(canvasContext, _) => {
      let context =
        Draw.createContext(
          ~canvasContext,
          ~width=Editor.visiblePixelWidth(editor),
          ~height=Editor.visiblePixelHeight(editor),
          ~editor,
          ~editorFont,
        );

      if (isActiveSplit) {
        let height = Editor.lineHeightInPixels(editor);
        let background = colors.cursorBackground;
        let foreground = colors.cursorForeground;

        if (!windowIsFocused) {
          let width = characterWidth;
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
        } else if (Vim.Mode.isInsert(mode)) {
          let width = 2.;
          Draw.rect(~context, ~x, ~y, ~width, ~height, ~color=foreground);
        } else {
          let width = characterWidth;
          Draw.rect(~context, ~x, ~y, ~width, ~height, ~color=foreground);

          editor
          |> Editor.getCharacterAtPosition(~position=cursorPosition)
          |> Option.iter(uchar =>
               if (!BufferViewTokenizer.isWhitespace(uchar)) {
                 let text = ZedBundled.make(1, uchar);
                 let font =
                   Service_Font.resolveWithFallback(
                     ~italic=false,
                     Revery.Font.Weight.Normal,
                     context.fontFamily,
                   );
                 let fontMetrics =
                   Revery.Font.getMetrics(font, context.fontSize);
                 Draw.shapedText(
                   ~context,
                   ~x=x -. 0.5,
                   ~y=characterPaddingY +. y -. fontMetrics.ascent -. 0.5,
                   ~bold=false,
                   ~italic=false,
                   ~color=
                     background |> Revery.Color.multiplyAlpha(textOpacity),
                   text,
                 );
               }
             );
        };
      };
    }}
  />;
};
