/*
 * WindowStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for window management
 */

module Core = Oni_Core;
module Model = Oni_Model;

open Model;
open Model.Actions;

module OptionEx = Core.Utility.OptionEx;

let start = () => {
  let quitEffect =
    Isolinear.Effect.createWithDispatch(~name="windows.quitEffect", dispatch =>
      dispatch(Model.Actions.Quit(false))
    );

  let windowUpdater = (state: Model.State.t, action: Model.Actions.t) =>
    switch (action) {
    | ViewCloseEditor(editorId) =>
      /* When an editor is closed... lets see if any window splits are empty */

      let editorGroups =
        Model.EditorGroups.closeEditor(~editorId, state.editorGroups);

      /* Remove splits */
      let layout =
        state.layout
        |> Feature_Layout.windows
        |> List.fold_left(
             (acc, editorGroupId) =>
               if (Model.EditorGroups.getEditorGroupById(
                     editorGroups,
                     editorGroupId,
                   )
                   == None) {
                 Feature_Layout.removeWindow(editorGroupId, acc);
               } else {
                 acc;
               },
             state.layout,
           );

      {...state, editorGroups, layout};

    | OpenFileByPath(_) => FocusManager.push(Editor, state)

    | _ => state
    };

  let updater = (state: Model.State.t, action: Model.Actions.t) => {
    let state = windowUpdater(state, action);

    let effect =
      switch (action) {
      // When opening a file, ensure that the active editor is getting focus
      | ViewCloseEditor(_) =>
        if (Feature_Layout.windows(state.layout) == []) {
          quitEffect;
        } else {
          Isolinear.Effect.none;
        }
      | _ => Isolinear.Effect.none
      };

    (state, effect);
  };

  updater;
};
