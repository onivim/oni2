/*
 * EditorGroups.re
 *
 * Managing an aggregate of EditorGroups
 */

open Oni_Core;
open Oni_Core.Utility;

type t = {
  idToGroup: IntMap.t(EditorGroup.t),
  activeId: int,
  /* Cache the last editor font, so when a new group is created, we can share it */
  lastEditorFont: option(Service_Font.font),
};

let create = () => {
  let defaultGroup = EditorGroup.create();
  let activeId = defaultGroup.editorGroupId;
  let idToGroup = IntMap.add(activeId, defaultGroup, IntMap.empty);

  {idToGroup, activeId, lastEditorFont: None};
};

let activeGroupId = model => model.activeId;

let add = (editorGroup, model) => {
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
};

let getAllVisibleEditors = ({idToGroup, _}) => {
  IntMap.fold(
    (_key, curr, acc) => {
      switch (EditorGroup.getActiveEditor(curr)) {
      | Some(editor) => [editor, ...acc]
      | None => acc
      }
    },
    idToGroup,
    [],
  );
};

let setActiveEditor = (~editorId, model) => {
  let (activeId, idToGroup) =
    IntMap.fold(
      (editorGroupId, editorGroup, acc) =>
        if (EditorGroup.hasEditor(~editorId, editorGroup)) {
          let (_, idToGroup) = acc;
          let newEditorGroup =
            EditorGroup.setActiveEditor(editorGroup, editorId);
          let idToGroup' =
            IntMap.add(editorGroup.editorGroupId, newEditorGroup, idToGroup);
          (editorGroupId, idToGroup');
        } else {
          acc;
        },
      model.idToGroup,
      (model.activeId, model.idToGroup),
    );

  {...model, activeId, idToGroup};
};

let getEditorGroupById = (model, id) => IntMap.find_opt(id, model.idToGroup);

let getFirstEditorGroup = ({idToGroup, _}) => {
  // TODO: Move 'remove' gesture inside EditorGroups, so that we can maintain
  // the invariant that there is _always_ at least one editor group.
  idToGroup |> IntMap.bindings |> List.hd |> snd;
};

let getActiveEditorGroup = model => getEditorGroupById(model, model.activeId);

let setActiveEditorGroup = (id, model) => {...model, activeId: id};

let isActive = (model, group: EditorGroup.t) =>
  group.editorGroupId == model.activeId;

let setBufferFont = (~bufferId, ~font, groups) => {
  let idToGroup =
    groups.idToGroup
    |> IntMap.map(group => EditorGroup.setBufferFont(~bufferId, ~font, group));

  {...groups, idToGroup};
};

/* Validate 'activeId' is set to a valid editor group,
   otherwise move to the first valid */
module Internal = {
  let applyToAllEditorGroups = (editors, action: Actions.t) =>
    IntMap.map(group => EditorGroupReducer.reduce(group, action), editors);

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

  let mapAndCleanEditorGroups = (f, editorGroups) => {
    let idToGroup = editorGroups.idToGroup |> IntMap.map(f);

    // Keep a handle on the active editor group - we should never
    // get completely empty!
    switch (IntMap.find_opt(editorGroups.activeId, idToGroup)) {
    // We shouldn't be in this state, ever
    | None => editorGroups
    | Some(activeEditorGroup) =>
      let idToGroup =
        idToGroup |> IntMap.filter((_, group) => !EditorGroup.isEmpty(group));

      let remainingGroupCount = List.length(IntMap.bindings(idToGroup));
      // We never let the editor groups get totally empty,
      // otherwise we run into the bad case of:
      // https://github.com/onivim/oni2/issues/733
      let idToGroup =
        if (remainingGroupCount == 0) {
          IntMap.add(
            activeEditorGroup.editorGroupId,
            activeEditorGroup,
            idToGroup,
          );
        } else {
          idToGroup;
        };

      {...editorGroups, idToGroup}
      // There's a chance the active group could've changed, so make sure
      // we point to one
      |> ensureActiveId;
    };
  };
};

let closeEditor = (~editorId, editorGroups) => {
  editorGroups
  |> Internal.mapAndCleanEditorGroups(group =>
       EditorGroup.removeEditorById(group, editorId)
     );
};

let closeBuffer = (~bufferId, editorGroups) => {
  editorGroups
  |> Internal.mapAndCleanEditorGroups(group =>
       EditorGroup.removeEditorsForBuffer(~bufferId, group)
     );
};

let reduce = (model, action: Actions.t) => {
  switch (action) {
  | EditorFont(Service_Font.FontLoaded(font)) => {
      ...model,
      idToGroup: Internal.applyToAllEditorGroups(model.idToGroup, action),
      lastEditorFont: Some(font),
    }

  | EditorSizeChanged(_) => {
      ...model,
      idToGroup: Internal.applyToAllEditorGroups(model.idToGroup, action),
    }

  | action =>
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
    }
  };
};

let getActiveEditor = editorGroups => {
  editorGroups.idToGroup
  |> IntMap.find_opt(editorGroups.activeId)
  |> OptionEx.flatMap(EditorGroup.getActiveEditor);
};

let updateEditor = (~editorId, msg, {idToGroup, _} as editorGroups) => {
  let (idToGroup', effects) =
    IntMap.fold(
      (key, curr, acc) => {
        let (groups, effects) = acc;

        let (group, maybeEffect) =
          curr |> EditorGroup.updateEditor(~editorId, msg);

        let groups' = IntMap.add(key, group, groups);
        let effects' =
          switch (maybeEffect) {
          | Some(eff) => [eff, ...effects]
          | None => effects
          };

        (groups', effects');
      },
      idToGroup,
      (idToGroup, []),
    );

  ({...editorGroups, idToGroup: idToGroup'}, effects);
};
