/*
 * EditorGroups.re
 *
 * Managing an aggregate of EditorGroups
 */

open Oni_Core;

type t = {
  idToGroup: IntMap.t(EditorGroup.t),
  activeId: int,
  /* Cache the last editor font, so when a new group is created, we can share it */
  lastEditorFont: option(Service_Font.t),
};

let create = () => {
  let defaultGroup = EditorGroup.create();
  let activeId = defaultGroup.editorGroupId;
  let idToGroup = IntMap.add(activeId, defaultGroup, IntMap.empty);

  {idToGroup, activeId, lastEditorFont: None};
};

let activeGroupId = model => model.activeId;

let getEditorGroupById = (model, id) => IntMap.find_opt(id, model.idToGroup);

let getActiveEditorGroup = model => getEditorGroupById(model, model.activeId);

let isActive = (model, group: EditorGroup.t) =>
  group.editorGroupId == model.activeId;

let applyToAllEditorGroups = (editors, action: Actions.t) =>
  IntMap.map(group => EditorGroupReducer.reduce(group, action), editors);

/* Validate 'activeId' is set to a valid editor group,
   otherwise move to the first valid */
let ensureActiveId = model => {
  switch (IntMap.find_opt(model.activeId, model.idToGroup)) {
  | Some(_) => model
  | None =>
    switch (IntMap.min_binding_opt(model.idToGroup)) {
    | Some((key, _)) => {...model, activeId: key}
    | _ => model
    }
  };
};

let isEmpty = (id, model) => {
  switch (IntMap.find_opt(id, model.idToGroup)) {
  | None => true
  | Some(group) => EditorGroup.isEmpty(group)
  };
};

let removeEmptyEditorGroups = model => {
  let idToGroup =
    IntMap.filter(
      (_, group) => !EditorGroup.isEmpty(group),
      model.idToGroup,
    );

  {...model, idToGroup};
};

let reduce = (model, action: Actions.t) => {
  switch (action) {
  | EditorFont(Service_Font.FontLoaded(font)) => {
      ...model,
      idToGroup: applyToAllEditorGroups(model.idToGroup, action),
      lastEditorFont: Some(font),
    }

  | WindowSetActive(_, editorGroupId) => {...model, activeId: editorGroupId}

  | EditorGroupAdd(editorGroup) =>
    let editorGroup =
      switch (model.lastEditorFont) {
      | Some(font) =>
        EditorGroupReducer.reduce(
          editorGroup,
          EditorFont(Service_Font.FontLoaded(font)),
        )
      | None => editorGroup
      };

    {
      ...model,
      activeId: editorGroup.editorGroupId,
      idToGroup:
        IntMap.add(editorGroup.editorGroupId, editorGroup, model.idToGroup),
    };

  | EditorGroupSetSize(editorGroupId, _) =>
    let idToGroup =
      IntMap.update(
        editorGroupId,
        editorGroup =>
          switch (editorGroup) {
          | Some(group) => Some(EditorGroupReducer.reduce(group, action))
          | None => None
          },
        model.idToGroup,
      );

    {...model, idToGroup};

  | action =>
    let newModel =
      switch (getActiveEditorGroup(model)) {
      | Some(group) => {
          ...model,
          idToGroup:
            IntMap.add(
              model.activeId,
              EditorGroupReducer.reduce(group, action),
              model.idToGroup,
            ),
        }
      | None => model
      };

    switch (action) {
    | ViewCloseEditor(_) =>
      newModel |> removeEmptyEditorGroups |> ensureActiveId
    | _ => newModel
    };
  };
};
