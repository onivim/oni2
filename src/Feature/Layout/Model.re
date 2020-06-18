type group = {
  id: int,
  items: list(Feature_Editor.Editor.t),
  selected: int,
};

type panel =
  | Left
  | Center
  | Bottom;

type model = {
  tree: Layout.t(int),
  uncommittedTree: [
    | `Resizing(Layout.t(int))
    | `Maximized(Layout.t(int))
    | `None
  ],
  groups: list(group),
  activeGroup: int,
};

let initial = {
  let initialGroup = {
    id: 0,
    items: [],
    selected: (-1) // TODO
  };

  {
    tree: Layout.singleton(initialGroup.id),
    uncommittedTree: `None,
    groups: [initialGroup],
    activeGroup: initialGroup.id,
  };
};

let activeTree = model =>
  switch (model.uncommittedTree) {
  | `Resizing(tree)
  | `Maximized(tree) => tree
  | `None => model.tree
  };

let updateTree = (f, model) => {
  ...model,
  tree: f(activeTree(model)),
  uncommittedTree: `None,
};

let windows = model => Layout.windows(activeTree(model));
let addWindow = (direction, focus) =>
  updateTree(Layout.addWindow(direction, focus));
let insertWindow = (target, direction, focus) =>
  updateTree(Layout.insertWindow(target, direction, focus));
let removeWindow = target => updateTree(Layout.removeWindow(target));

let move = (focus, dirX, dirY, layout) => {
  let positioned = Positioned.fromLayout(0, 0, 200, 200, layout);

  Positioned.move(focus, dirX, dirY, positioned)
  |> Option.value(~default=focus);
};

let moveLeft = current => move(current, -1, 0);
let moveRight = current => move(current, 1, 0);
let moveUp = current => move(current, 0, -1);
let moveDown = current => move(current, 0, 1);

let openEditor = (editor, model) => {
  open Feature_Editor;

  let bufferId = Editor.getBufferId(editor);

  let openInGroup = group =>
    switch (
      List.find_opt(e => Editor.getBufferId(e) == bufferId, group.items)
    ) {
    | Some(editor) => {...group, selected: Editor.getId(editor)}
    | None => {
        ...group,
        items: [editor, ...group.items],
        selected: Editor.getId(editor),
      }
    };

  {
    ...model,
    groups:
      List.map(
        group => group.id == model.activeGroup ? openInGroup(group) : group,
        model.groups,
      ),
  };
};
