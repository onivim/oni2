open EditorCoreTypes;
open Revery.UI;

open Oni_Core;

module Log = (val Log.withNamespace("Oni2.UI.EditorSurface"));

module FontIcon = Oni_Components.FontIcon;
module BufferHighlights = Oni_Syntax.BufferHighlights;
module Completions = Feature_LanguageSupport.Completions;
module Diagnostics = Feature_LanguageSupport.Diagnostics;
module Diagnostic = Feature_LanguageSupport.Diagnostic;
module Definition = Feature_LanguageSupport.Definition;

module Styles = {
  open Style;

  let bufferViewOverlay = [
    pointerEvents(`Ignore),
    position(`Absolute),
    top(0),
    left(0),
    right(0),
    bottom(0),
  ];
};

let completionsView =
    (
      ~completions,
      ~cursorPixelX,
      ~cursorPixelY,
      ~colors,
      ~theme,
      ~tokenTheme,
      ~editorFont: Service_Font.font,
      (),
    ) =>
  Completions.isActive(completions)
    ? <CompletionsView
        x=cursorPixelX
        y=cursorPixelY
        lineHeight={editorFont.measuredHeight}
        colors
        theme
        tokenTheme
        editorFont
        completions
      />
    : React.empty;

let make =
    (
      ~buffer,
      ~isActiveSplit,
      ~cursorPosition: Location.t,
      ~editor: Editor.t,
      ~gutterWidth,
      ~completions,
      ~colors,
      ~theme,
      ~tokenTheme,
      ~editorFont: Service_Font.font,
      (),
    ) => {
  let cursorOffset = Editor.getCursorOffset(~buffer, ~cursorPosition);

  let cursorPixelY =
    int_of_float(
      editorFont.measuredHeight
      *. float(Index.toZeroBased(cursorPosition.line))
      -. editor.scrollY
      +. 0.5,
    );

  let cursorPixelX =
    int_of_float(
      gutterWidth
      +. editorFont.measuredWidth
      *. float(cursorOffset)
      -. editor.scrollX
      +. 0.5,
    );

  isActiveSplit
    ? <View style=Styles.bufferViewOverlay>
        <completionsView
          completions
          cursorPixelX
          cursorPixelY
          colors
          theme
          tokenTheme
          editorFont
        />
      </View>
    : React.empty;
};
