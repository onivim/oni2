open EditorCoreTypes;
open Revery.UI;

open Oni_Core;

module Log = (val Log.withNamespace("Oni2.UI.EditorSurface"));

module FontIcon = Oni_Components.FontIcon;
module BufferHighlights = Oni_Syntax.BufferHighlights;
module Diagnostics = Feature_LanguageSupport.Diagnostics;
module Diagnostic = Feature_LanguageSupport.Diagnostic;

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
      ~languageSupport,
      ~cursorPixelX,
      ~cursorPixelY,
      ~theme,
      ~tokenTheme,
      ~editorFont: Service_Font.font,
      (),
    ) =>
  Feature_LanguageSupport.Completion.isActive(languageSupport)
    ? <Feature_LanguageSupport.Completion.View
        x=cursorPixelX
        y=cursorPixelY
        lineHeight={editorFont.measuredHeight}
        theme
        tokenTheme
        editorFont
        //colors
        model=languageSupport
      />
    : React.empty;

let make =
    (
      ~isActiveSplit,
      ~cursorPosition: CharacterPosition.t,
      ~editor: Editor.t,
      ~gutterWidth,
      ~theme,
      ~tokenTheme,
      ~languageSupport,
      ~editorFont: Service_Font.font,
      (),
    ) => {
  let ({pixelX, pixelY}: Editor.pixelPosition, _) =
    Editor.bufferCharacterPositionToPixel(~position=cursorPosition, editor);

  let cursorPixelY = pixelY |> int_of_float;
  let cursorPixelX = pixelX +. gutterWidth |> int_of_float;

  isActiveSplit
    ? <View style=Styles.bufferViewOverlay>
        <completionsView
          languageSupport
          cursorPixelX
          cursorPixelY
          theme
          tokenTheme
          editorFont
        />
      </View>
    : React.empty;
};
