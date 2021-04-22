open Revery.UI;

let make =
    (
      ~config,
      ~isFocused: bool,
      ~iconTheme,
      ~languageInfo,
      ~model,
      ~decorations,
      ~theme,
      ~font,
      ~expanded,
      ~onRootClicked: unit => unit,
      ~dispatch: Model.msg => unit,
      (),
    ) => {
  switch ((model: Model.model)) {
  | {tree: Some(_), treeView, active, rootName, _} =>
    let focusedIndex = Model.getFocusedIndex(model);
    <FileTreeView
      config
      rootName
      isFocused
      iconTheme
      languageInfo
      focusedIndex
      decorations
      active
      treeView
      theme
      font
      expanded
      onRootClicked
      dispatch
    />;
  | _ => React.empty
  };
};
