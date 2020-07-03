[@deriving show]
type t =
  | All
  | Editor(int);

let matches = (editor, scope) => {
  switch (scope) {
  | All => true
  | Editor(id) => id == Feature_Editor.Editor.getId(editor)
  };
};
