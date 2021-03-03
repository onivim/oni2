open EditorCoreTypes;
open Revery.UI;

open Oni_Core;

module Log = (val Log.withNamespace("Oni2.UI.EditorSurface"));

module FontIcon = Oni_Components.FontIcon;
module Diagnostics = Feature_Diagnostics;
module Diagnostic = Feature_Diagnostics.Diagnostic;

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

let signatureHelpView =
    (
      ~cursorPixelX,
      ~cursorPixelY,
      ~languageSupport,
      ~theme,
      ~tokenTheme,
      ~editorFont: Service_Font.font,
      ~uiFont: UiFont.t,
      ~languageInfo,
      ~buffer,
      ~grammars,
      ~dispatch,
      (),
    ) =>
  Feature_LanguageSupport.SignatureHelp.isActive(languageSupport)
    ? <Feature_LanguageSupport.SignatureHelp.View
        x=cursorPixelX
        y=cursorPixelY
        theme
        tokenTheme
        editorFont
        uiFont
        languageInfo
        buffer
        grammars
        dispatch
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
      ~uiFont,
      ~buffer,
      ~languageInfo,
      ~grammars,
      (),
    ) => {
  let ({x: pixelX, y: pixelY}: PixelPosition.t, _) =
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
        <signatureHelpView
          languageSupport
          cursorPixelX
          cursorPixelY
          theme
          tokenTheme
          editorFont
          uiFont
          buffer
          languageInfo
          dispatch={_ => ()}
          grammars
        />
      </View>
    : React.empty;
};
