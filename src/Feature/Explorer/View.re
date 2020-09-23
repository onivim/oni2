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

    <FileTreeView
      isFocused
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
