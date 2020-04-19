open Revery;

module Colors = Feature_Theme.Colors;
open Colors;

type t = {
  editorBackground: Color.t,
  editorForeground: Color.t,
  selectionBackground: Color.t,
  findMatchBackground: Color.t,
  cursorBackground: Color.t,
  cursorForeground: Color.t,
  gutterBackground: Color.t,
  gutterModifiedBackground: Color.t,
  gutterAddedBackground: Color.t,
  gutterDeletedBackground: Color.t,
  indentGuideBackground: Color.t,
  indentGuideActiveBackground: Color.t,
  lineNumberForeground: Color.t,
  lineNumberActiveForeground: Color.t,
  lineHighlightBackground: Color.t,
  overviewRulerBracketMatchForeground: Color.t,
  rulerForeground: Color.t,
  suggestWidgetBorder: Color.t,
  suggestWidgetBackground: Color.t,
  suggestWidgetSelectedBackground: Color.t,
  hoverWidgetBackground: Color.t,
  hoverWidgetForeground: Color.t,
  hoverWidgetBorder: Color.t,
  whitespaceForeground: Color.t,
  scrollbarSliderBackground: Color.t,
  scrollbarSliderHoverBackground: Color.t,
  normalModeBackground: Color.t,
  // Minimap
  minimapSliderBackground: Color.t,
  minimapSelectionHighlight: Color.t,
};

let precompute = theme => {
  editorBackground: Editor.background.from(theme),
  editorForeground: Editor.foreground.from(theme),
  selectionBackground: Editor.selectionBackground.from(theme),
  findMatchBackground: Editor.findMatchBackground.from(theme),
  lineHighlightBackground: Editor.lineHighlightBackground.from(theme),
  cursorBackground: EditorCursor.background.from(theme),
  cursorForeground: EditorCursor.foreground.from(theme),
  gutterBackground: EditorGutter.background.from(theme),
  indentGuideBackground: EditorIndentGuide.background.from(theme),
  indentGuideActiveBackground: EditorIndentGuide.activeBackground.from(theme),
  lineNumberForeground: EditorLineNumber.foreground.from(theme),
  lineNumberActiveForeground: EditorLineNumber.activeForeground.from(theme),
  overviewRulerBracketMatchForeground:
    EditorOverviewRuler.bracketMatchForeground.from(theme),
  rulerForeground: EditorRuler.foreground.from(theme),
  gutterModifiedBackground: EditorGutter.modifiedBackground.from(theme),
  gutterAddedBackground: EditorGutter.addedBackground.from(theme),
  gutterDeletedBackground: EditorGutter.deletedBackground.from(theme),
  suggestWidgetBorder: EditorSuggestWidget.border.from(theme),
  suggestWidgetBackground: EditorSuggestWidget.background.from(theme),
  suggestWidgetSelectedBackground:
    EditorSuggestWidget.selectedBackground.from(theme),
  hoverWidgetBackground: EditorHoverWidget.background.from(theme),
  hoverWidgetForeground: EditorHoverWidget.foreground.from(theme),
  hoverWidgetBorder: EditorHoverWidget.border.from(theme),
  whitespaceForeground: EditorWhitespace.foreground.from(theme),
  scrollbarSliderBackground: ScrollbarSlider.background.from(theme),
  scrollbarSliderHoverBackground: ScrollbarSlider.hoverBackground.from(theme),
  normalModeBackground: Oni.normalModeBackground.from(theme),
  // Minimap
  minimapSliderBackground: MinimapSlider.background.from(theme),
  minimapSelectionHighlight: Colors.Minimap.selectionHighlight.from(theme),
};
