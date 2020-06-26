module Local = {
  module Configuration = Configuration;
};
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
  let replaceAllWith: (Editor.t, t) => t;
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

  let replaceAllWith = (editor, group) => {
    ...group,
    editors: [editor],
    selectedId: Editor.getId(editor),
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

type layout = {
  tree: Layout.t(int),
  uncommittedTree: [
    | `Resizing(Layout.t(int))
    | `Maximized(Layout.t(int))
    | `None
  ],
  groups: list(Group.t),
  activeGroupId: int,
};

let activeTree = layout =>
  switch (layout.uncommittedTree) {
  | `Resizing(tree)
  | `Maximized(tree) => tree
  | `None => layout.tree
  };

type model = {
  layouts: list(layout),
  activeLayoutIndex: int,
};

let initial = editors => {
  let initialGroup = Group.create(editors);

  let initialLayout = {
    tree: Layout.singleton(initialGroup.id),
    uncommittedTree: `None,
    groups: [initialGroup],
    activeGroupId: initialGroup.id,
  };

  {layouts: [initialLayout], activeLayoutIndex: 0};
};

let groupById = (id, layout) =>
  List.find_opt((group: Group.t) => group.id == id, layout.groups);

let activeLayout = model => List.nth(model.layouts, model.activeLayoutIndex);

let activeGroup = model =>
  model
  |> activeLayout
  |> (layout => groupById(layout.activeGroupId, layout) |> Option.get);

let activeEditor = model => model |> activeGroup |> Group.selected;

let updateActiveLayout = (f, model) => {
  ...model,
  layouts:
    List.mapi(
      (i, layout) => i == model.activeLayoutIndex ? f(layout) : layout,
      model.layouts,
    ),
};

let updateTree = f =>
  updateActiveLayout(layout =>
    {...layout, tree: f(activeTree(layout)), uncommittedTree: `None}
  );

let updateGroup = (id, f, layout) => {
  ...layout,
  groups:
    List.map(
      (group: Group.t) => group.id == id ? f(group) : group,
      layout.groups,
    ),
};

let updateActiveGroup = f =>
  updateActiveLayout(layout => updateGroup(layout.activeGroupId, f, layout));

let windows = model => Layout.windows(model |> activeLayout |> activeTree);

let visibleEditors = model =>
  model
  |> windows
  |> List.filter_map(id => model |> activeLayout |> groupById(id))
  |> List.map(Group.selected);

let editorById = (id, model) =>
  Base.List.find_map(activeLayout(model).groups, ~f=group =>
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

  updateActiveLayout(
    layout =>
      {
        groups: [newGroup, ...layout.groups],
        activeGroupId: newGroup.id,
        tree:
          Layout.insertWindow(
            `After(layout.activeGroupId),
            direction,
            newGroup.id,
            activeTree(layout),
          ),
        uncommittedTree: `None,
      },
    model,
  );
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

let nextEditor = updateActiveGroup(Group.nextEditor);

let previousEditor = updateActiveGroup(Group.previousEditor);

let removeLayoutTab = (index, model) => {
  let left = Base.List.take(model.layouts, index);
  let right = Base.List.drop(model.layouts, index + 1);
  let layouts = left @ right;

  if (layouts == []) {
    None;
  } else {
    Some({
      layouts,
      activeLayoutIndex:
        min(model.activeLayoutIndex, List.length(layouts) - 1),
    });
  };
};

let removeActiveLayoutTab = model =>
  removeLayoutTab(model.activeLayoutIndex, model);

let removeOtherLayoutTabs = model => {
  {layouts: [activeLayout(model)], activeLayoutIndex: 0};
};

let removeEditor = (editorId, model) => {
  let removeFromLayout = layout => {
    let groups =
      List.filter_map(
        (group: Group.t) =>
          group.id == layout.activeGroupId
            ? Group.removeEditor(editorId, group) : Some(group),
        layout.groups,
      );

    if (groups == []) {
      None; // Group was removed, no groups left. Abort! Abort!
    } else if (List.length(groups) != List.length(layout.groups)) {
      // Group was removed, remove from tree and make another active

      let tree =
        Layout.removeWindow(layout.activeGroupId, activeTree(layout));

      let activeGroupId =
        switch (
          ListEx.findIndex(
            (g: Group.t) => g.id == layout.activeGroupId,
            layout.groups,
          )
        ) {
        | Some(0) => List.hd(groups).id
        | Some(i) => List.nth(groups, i - 1).id
        | None => layout.activeGroupId
        };

      Some({...layout, tree, groups, activeGroupId});
    } else {
      Some({...layout, groups});
    };
  };

  switch (removeFromLayout(activeLayout(model))) {
  | Some(newLayout) =>
    Some({
      ...model,
      layouts:
        List.mapi(
          (i, layout) => i == model.activeLayoutIndex ? newLayout : layout,
          model.layouts,
        ),
    })

  | None => removeLayoutTab(model.activeLayoutIndex, model)
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

let addLayoutTab = model => {
  let newEditor = activeEditor(model) |> Editor.copy;
  let newGroup = Group.create([newEditor]);

  let newLayout = {
    tree: Layout.singleton(newGroup.id),
    uncommittedTree: `None,
    groups: [newGroup],
    activeGroupId: newGroup.id,
  };

  let left = Base.List.take(model.layouts, model.activeLayoutIndex + 1);
  let right = Base.List.drop(model.layouts, model.activeLayoutIndex + 1);

  {
    layouts: left @ [newLayout] @ right,
    activeLayoutIndex: model.activeLayoutIndex + 1,
  };
};

let gotoLayoutTab = (index, model) => {
  ...model,
  activeLayoutIndex:
    IntEx.clamp(index, ~lo=0, ~hi=List.length(model.layouts) - 1),
};

let previousLayoutTab = (~count=1, model) =>
  gotoLayoutTab(model.activeLayoutIndex - count, model);

let nextLayoutTab = (~count=1, model) =>
  gotoLayoutTab(model.activeLayoutIndex + count, model);

let moveActiveLayoutTabTo = (index, model) => {
  let newLayouts =
    ListEx.move(~fromi=model.activeLayoutIndex, ~toi=index, model.layouts);

  {
    layouts: newLayouts,
    activeLayoutIndex:
      IntEx.clamp(index, ~lo=0, ~hi=List.length(newLayouts) - 1),
  };
};

let map = (f, model) => {
  ...model,
  layouts:
    List.map(
      layout => {...layout, groups: List.map(Group.map(f), layout.groups)},
      model.layouts,
    ),
};
