open Revery.UI;

let make =
    (
      ~isFocused: bool,
      ~iconTheme,
      ~languageInfo,
      ~model,
      ~decorations,
      ~theme,
      ~font,
      ~dispatch: Model.msg => unit,
      (),
    ) => {
  switch ((model: Model.model)) {
  | {tree: Some(_), treeView, active, _} =>
    let focusedIndex = Model.getFocusedIndex(model);
    <FileTreeView
      isFocused
      iconTheme
      languageInfo
      focusedIndex
      decorations
      active
      treeView
      theme
      font
      dispatch
    />;
  | _ => React.empty
  };
};
