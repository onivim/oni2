open Oni_Core;

let rightClick = {
  open ContextMenu.Schema;

  let menu = menu(~uniqueId="editor.rightClick", ~parent=None, "Right Click");

  let group =
    group(~parent=menu, [Feature_Clipboard.Contributions.MenuItems.paste]);

  union([menu] |> menus, [group] |> groups);
};
