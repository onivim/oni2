type model;

let initial: model;

module View: {
  let make:
    (
      ~colorTheme: Oni_Core.ColorTheme.Colors.t,
      ~tokenTheme: Oni_Syntax.TokenTheme.t,
      ~languageInfo: Exthost.LanguageInfo.t,
      ~uiFont: Oni_Core.UiFont.t,
      ~editorFont: Service_Font.font,
      ~model: model,
      ~editor: Feature_Editor.Editor.t,
      ~buffer: Oni_Core.Buffer.t,
      ~gutterWidth: float,
      ~grammars: Oni_Syntax.GrammarRepository.t,
      ~diagnostics: Feature_LanguageSupport.Diagnostics.t,
      unit
    ) =>
    Revery.UI.element;
};
