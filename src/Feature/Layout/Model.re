open Oni_Core;
open Utility;
open Feature_Editor;

module Group: {
  type t =
    pri {
      id: int,
      editors: list(Editor.t),
      selectedId: int,
    };

  let create: list(Editor.t) => t;

  let selected: t => Editor.t;

  let select: (int, t) => t;
  let nextEditor: t => t;
  let previousEditor: t => t;
  let openEditor: (Editor.t, t) => t;
  let removeEditor: (int, t) => option(t);

  let map: (Editor.t => Editor.t, t) => t;
} = {
  type t = {
    id: int,
    editors: list(Editor.t),
    selectedId: int,
  };

  let create = {
    let lastId = ref(-1);

    editors => {
      assert(editors != []);

      incr(lastId);

      let selectedId = editors |> ListEx.last |> Option.get |> Editor.getId;
      {id: lastId^, editors, selectedId};
    };
  };

  let select = (id, group) => {
    assert(List.exists(item => Editor.getId(item) == id, group.editors));

    {...group, selectedId: id};
  };

  let nextEditor = group => {
    let rec loop =
      fun
      | []
      | [_] => group.selectedId
      | [next, current, ..._] when Editor.getId(current) == group.selectedId =>
        Editor.getId(next)
      | [_, ...rest] => loop(rest);

    {...group, selectedId: loop(group.editors)};
  };

  let previousEditor = group => {
    let rec loop =
      fun
      | []
      | [_] => group.selectedId
      | [current, previous, ..._]
          when Editor.getId(current) == group.selectedId =>
        Editor.getId(previous)
      | [_, ...rest] => loop(rest);

    {...group, selectedId: loop(group.editors)};
  };

  let selected = group =>
    List.find(
      editor => Editor.getId(editor) == group.selectedId,
      group.editors,
    );

  let openEditor = (editor, group) => {
    let bufferId = Editor.getBufferId(editor);
    switch (
      List.find_opt(e => Editor.getBufferId(e) == bufferId, group.editors)
    ) {
    | Some(editor) => {...group, selectedId: Editor.getId(editor)}
    | None => {
        ...group,
        editors: [editor, ...group.editors],
        selectedId: Editor.getId(editor),
      }
    };
  };

  let removeEditor = (editorId, group) => {
    switch (List.filter(e => Editor.getId(e) != editorId, group.editors)) {
    | [] => None
    | editors =>
      let selectedId =
        if (group.selectedId == editorId) {
          switch (
            ListEx.findIndex(e => Editor.getId(e) == editorId, group.editors)
          ) {
          | Some(0) => List.hd(editors) |> Editor.getId
          | Some(i) => List.nth(editors, i - 1) |> Editor.getId
          | None => group.selectedId
          };
        } else {
          group.selectedId;
        };

      Some({...group, editors, selectedId});
    };
  };

  let map = (f, group) => {...group, editors: List.map(f, group.editors)};
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
  activeGroupId: int,
};

let initial = editors => {
  let initialGroup = Group.create(editors);

  {
    tree: Layout.singleton(initialGroup.id),
    uncommittedTree: `None,
    groups: [initialGroup],
    activeGroupId: initialGroup.id,
  };
};

let groupById = (id, model) =>
  List.find_opt((group: Group.t) => group.id == id, model.groups);

let activeGroup = model =>
  groupById(model.activeGroupId, model) |> Option.get;

let activeEditor = model => model |> activeGroup |> Group.selected;

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

let updateActiveGroup = (f, model) => {
  ...model,
  groups:
    List.map(
      (group: Group.t) => group.id == model.activeGroupId ? f(group) : group,
      model.groups,
    ),
};

let windows = model => Layout.windows(activeTree(model));

let visibleEditors = model =>
  model
  |> windows
  |> List.filter_map(id => groupById(id, model))
  |> List.map(Group.selected);

let editorById = (id, model) =>
  Base.List.find_map(model.groups, ~f=group =>
    List.find_opt(editor => Editor.getId(editor) == id, group.editors)
  );

let addWindow = (direction, focus) =>
  updateTree(Layout.addWindow(direction, focus));
let insertWindow = (target, direction, focus) =>
  updateTree(Layout.insertWindow(target, direction, focus));
let removeWindow = target => updateTree(Layout.removeWindow(target));

let split = (direction, model) => {
  let activeEditor = activeEditor(model);
  let newGroup = Group.create([Editor.copy(activeEditor)]);

  {
    groups: [newGroup, ...model.groups],
    activeGroupId: newGroup.id,
    tree:
      Layout.insertWindow(
        `After(model.activeGroupId),
        direction,
        newGroup.id,
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

let nextEditor = model => updateActiveGroup(Group.nextEditor, model);

let previousEditor = model => updateActiveGroup(Group.previousEditor, model);

let openEditor = (editor, model) =>
  updateActiveGroup(Group.openEditor(editor), model);

let removeEditor = (editorId, model) => {
  let groups =
    List.filter_map(
      (group: Group.t) =>
        group.id == model.activeGroupId
          ? Group.removeEditor(editorId, group) : Some(group),
      model.groups,
    );

  if (groups == []) {
    None; // Group was removed, no groups left. Abort! Abort!
  } else if (List.length(groups) != List.length(model.groups)) {
    // Group was removed, remove from tree and make another active

    let tree = Layout.removeWindow(model.activeGroupId, activeTree(model));

    let activeGroupId =
      switch (
        ListEx.findIndex(
          (g: Group.t) => g.id == model.activeGroupId,
          model.groups,
        )
      ) {
      | Some(0) => List.hd(groups).id
      | Some(i) => List.nth(groups, i - 1).id
      | None => model.activeGroupId
      };

    Some({...model, tree, groups, activeGroupId});
  } else {
    Some({...model, groups});
  };
};

let removeActiveEditor = model => {
  let activeEditorId = model |> activeEditor |> Editor.getId;
  removeEditor(activeEditorId, model);
};

let closeBuffer = (~force, buffer, model) => {
  let activeEditor = activeEditor(model);
  let activeEditorId = Editor.getId(activeEditor);
  let bufferMeta = Vim.BufferMetadata.ofBuffer(buffer);

  if (Editor.getBufferId(activeEditor) == bufferMeta.id
      && (force || !bufferMeta.modified)) {
    removeEditor(activeEditorId, model);
  } else {
    Some(model);
  };
};

let map = (f, model) => {
  ...model,
  groups: List.map(Group.map(f), model.groups),
};
