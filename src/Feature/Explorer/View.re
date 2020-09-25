open Revery.UI;

let make =
    (
      ~isFocused: bool,
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
