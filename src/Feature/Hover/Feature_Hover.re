/* Feature_Hover.re
     This feature project contains logic related to Hover
   */
open Oni_Core;
open Revery;
open Revery.UI;
open Revery.UI.Components;
open EditorCoreTypes;

module Log = (val Log.withNamespace("Oni.Feature.Hover"));

[@deriving show({with_path: false})]
type provider = {
  handle: int,
  selector: list(Exthost.DocumentFilter.t),
};

type model = {
  shown: bool,
  providers: list(provider),
  contents: list(Exthost.MarkdownString.t),
  range: option(EditorCoreTypes.Range.t),
  triggeredFrom:
    option([ | `CommandPalette | `Mouse(EditorCoreTypes.Location.t)]),
  lastRequestID: option(int),
  editorID: option(int),
};

let initial = {
  shown: false,
  providers: [],
  contents: [],
  range: None,
  triggeredFrom: None,
  lastRequestID: None,
  editorID: None,
};

module Styles = {
  open Style;
  module Colors = Feature_Theme.Colors;

  let diagnostic = (~theme) => [
    textOverflow(`Ellipsis),
    color(Colors.Editor.foreground.from(theme)),
    backgroundColor(Colors.EditorHoverWidget.background.from(theme)),
  ];

  let hr = (~theme) => [
    flexGrow(1),
    height(1),
    backgroundColor(Colors.EditorHoverWidget.border.from(theme)),
    marginTop(3),
    marginBottom(3),
  ];
};

module View = {
  let horizontalRule = (~theme, ()) =>
    <Row> <View style={Styles.hr(~theme)} /> </Row>;

  let hover =
      (
        ~x,
        ~y,
        ~colorTheme,
        ~tokenTheme,
        ~languageInfo,
        ~uiFont: UiFont.t,
        ~editorFont: Service_Font.font,
        ~model,
        ~grammars,
        ~diagnostic,
        ~buffer,
        ~editor,
        (),
      ) => {
    let defaultLanguage =
      Option.value(
        ~default=Exthost.LanguageInfo.defaultLanguage,
        Buffer.getFileType(buffer),
      );

    let hoverMarkdown = (~markdown) =>
      Oni_Components.Markdown.make(
        ~colorTheme,
        ~tokenTheme,
        ~languageInfo,
        ~defaultLanguage,
        ~fontFamily={
          uiFont.family;
        },
        ~codeFontFamily={
          editorFont.fontFamily;
        },
        ~grammars,
        ~markdown=Exthost.MarkdownString.toString(markdown),
        ~baseFontSize=uiFont.size,
        ~codeBlockStyle=Style.[flexGrow(1)],
        ~codeBlockFontSize=editorFont.fontSize,
      );

    let hoverDiagnostic =
        (~diagnostic: Feature_LanguageSupport.Diagnostic.t, ()) => {
      <Text
        text={diagnostic.message}
        fontFamily={editorFont.fontFamily}
        fontSize={editorFont.fontSize}
        style={Styles.diagnostic(~theme=colorTheme)}
      />;
    };
    switch (model.editorID) {
    | Some(editorID) when editorID == Feature_Editor.Editor.getId(editor) =>
      <Oni_Components.HoverView x y theme=colorTheme>
        {List.map(markdown => <hoverMarkdown markdown />, model.contents)
         |> React.listToElement}
        {model.contents != [] && diagnostic != []
           ? <horizontalRule theme=colorTheme /> : React.empty}
        {List.map(diag => <hoverDiagnostic diagnostic=diag />, diagnostic)
         |> React.listToElement}
      </Oni_Components.HoverView>
    | _ => React.empty
    };
  };

  let make =
      (
        ~colorTheme,
        ~tokenTheme,
        ~languageInfo,
        ~uiFont: UiFont.t,
        ~editorFont: Service_Font.font,
        ~model,
        ~editor: Feature_Editor.Editor.t,
        ~buffer,
        ~gutterWidth,
        ~grammars,
        ~diagnostics,
        (),
      ) => {
    let (maybeCoords, maybeDiagnostic): (
      option((int, int)),
      option(list(Feature_LanguageSupport.Diagnostic.t)),
    ) =
      switch (model.range, model.triggeredFrom, model.shown) {
      | (Some(range), Some(trigger), true) =>
        let diagLocation =
          switch (trigger) {
          | `Mouse(location) => location
          | `CommandPalette => Feature_Editor.Editor.getPrimaryCursor(editor)
          };

        let diagnostic =
          Feature_LanguageSupport.Diagnostics.getDiagnosticsAtPosition(
            diagnostics,
            buffer,
            diagLocation,
          );

        let hoverLocation =
          switch (diagnostic) {
          | [] => range.start
          | [diag, ..._] => diag.range.start
          };

        let ({pixelX, pixelY}: Feature_Editor.Editor.pixelPosition, _) =
          Feature_Editor.Editor.bufferLineCharacterToPixel(
            ~line=hoverLocation.line |> Index.toZeroBased,
            ~characterIndex=hoverLocation.column |> Index.toZeroBased,
            editor,
          );

        let x = int_of_float(pixelX +. gutterWidth);
        let y =
          int_of_float(
            pixelY +. Feature_Editor.Editor.lineHeightInPixels(editor),
          );

        // TODO: Hover width?

        (Some((x, y)), Some(diagnostic));
      | (None, Some(trigger), true) =>
        let location =
          switch (trigger) {
          | `Mouse(location) => location
          | `CommandPalette => Feature_Editor.Editor.getPrimaryCursor(editor)
          };

        let ({pixelX, pixelY}: Feature_Editor.Editor.pixelPosition, _) =
          Feature_Editor.Editor.bufferLineCharacterToPixel(
            ~line=location.line |> Index.toZeroBased,
            ~characterIndex=location.column |> Index.toZeroBased,
            editor,
          );

        let x = int_of_float(pixelX +. gutterWidth);
        let y =
          int_of_float(
            pixelY +. Feature_Editor.Editor.lineHeightInPixels(editor),
          );

        let diagnostic =
          Feature_LanguageSupport.Diagnostics.getDiagnosticsAtPosition(
            diagnostics,
            buffer,
            location,
          );

        diagnostic == [] ? (None, None) : (Some((x, y)), Some(diagnostic));
      | _ => (None, None)
      };
    switch (maybeCoords, maybeDiagnostic) {
    | (Some((x, y)), Some(diagnostic)) =>
      <hover
        x
        y
        colorTheme
        tokenTheme
        languageInfo
        uiFont
        editorFont
        model
        grammars
        diagnostic
        buffer
        editor
      />
    | _ => React.empty
    };
  };
};
