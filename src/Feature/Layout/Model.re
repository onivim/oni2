open Feature_Editor;

module Group: {
  type t =
    pri {
      id: int,
      items: list(Editor.t),
      selected: int,
    };

  let empty: t;
  let create: Editor.t => t;

  let selected: t => Editor.t;

  let select: (int, t) => t;
  let openEditor: (Editor.t, t) => t;

  let map: (Editor.t => Editor.t, t) => t;
} = {
  type t = {
    id: int,
    items: list(Editor.t),
    selected: int,
  };

  // TODO: remove
  let empty = {id: (-1), items: [], selected: (-1)};

  let create = {
    let lastId = ref(-1);

    editor => {
      incr(lastId);

      {id: lastId^, items: [editor], selected: Editor.getId(editor)};
    };
  };

  let select = (id, group) => {
    assert(List.exists(item => Editor.getId(item) == id, group.items));

    {...group, selected: id};
  };

  let selected = group =>
    List.find(editor => Editor.getId(editor) == group.selected, group.items);

  let openEditor = (editor, group) => {
    let bufferId = Editor.getBufferId(editor);
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
  };

  let map = (f, group) => {...group, items: List.map(f, group.items)};
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
  groups: list(Group.t),
  activeGroup: int,
};

let initial = {
  let initialGroup = Group.empty;

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

let split = (direction, model) => {
  let activeGroup =
    List.find(
      (group: Group.t) => group.id == model.activeGroup,
      model.groups,
    );
  let activeEditor = Group.selected(activeGroup);
  let group = Group.create(activeEditor);

  {
    ...model,
    groups: [group, ...model.groups],
    tree:
      Layout.insertWindow(
        `After(model.activeGroup),
        direction,
        group.id,
        activeTree(model),
      ),
    uncommittedTree: `None,
  };
};

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
  {
    ...model,
    groups:
      List.map(
        (group: Group.t) =>
          group.id == model.activeGroup
            ? Group.openEditor(editor, group) : group,
        model.groups,
      ),
  };
};

let map = (f, model) => {
  ...model,
  groups: List.map(Group.map(f), model.groups),
};
