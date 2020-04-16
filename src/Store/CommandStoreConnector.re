open Oni_Core;

open Oni_Model;
open Oni_Model.Actions;

module KeyDisplayer = Oni_Components.KeyDisplayer;

module Constants = {
  let zoomStep = 0.2;
  let defaultZoomValue = 1.0;
  let minZoomValue = 0.0;
  let maxZoomValue = 2.8;
};

let start = () => {
  let closeEditorEffect = (state, _) =>
    Isolinear.Effect.createWithDispatch(~name="closeEditorEffect", dispatch => {
      let editor =
        Selectors.getActiveEditorGroup(state) |> Selectors.getActiveEditor;

      switch (editor) {
      | None => ()
      | Some(v) => dispatch(ViewCloseEditor(v.editorId))
      };
    });

  let splitEditorEffect = (state, direction, _) =>
    Isolinear.Effect.createWithDispatch(~name="splitEditorEffect", dispatch => {
      let buffer = Selectors.getActiveBuffer(state);

      let newEditorGroup =
        switch (buffer) {
        | Some(b) =>
          let ec = EditorGroup.create();
          let (g, editorId) =
            EditorGroup.getOrCreateEditorForBuffer(
              ~font=state.editorFont,
              ~bufferId=Buffer.getId(b),
              ec,
            );
          let g = EditorGroup.setActiveEditor(g, editorId);
          g;
        | None => EditorGroup.create()
        };

      dispatch(EditorGroupAdd(newEditorGroup));

      let split =
        Feature_Layout.WindowTree.createSplit(
          ~editorGroupId=newEditorGroup.editorGroupId,
          (),
        );

      dispatch(AddSplit(direction, split));
    });

  let windowMoveEffect = (state: State.t, direction, _) => {
    Isolinear.Effect.createWithDispatch(~name="window.move", dispatch => {
      let windowId = Feature_Layout.move(direction, state.layout);
      let maybeEditorGroupId =
        Feature_Layout.WindowTree.getEditorGroupIdFromSplitId(
          windowId,
          state.layout.windowTree,
        );

      switch (maybeEditorGroupId) {
      | Some(editorGroupId) =>
        dispatch(WindowSetActive(windowId, editorGroupId))
      | None => ()
      };
    });
  };

  let togglePathEffect = name =>
    Isolinear.Effect.create(
      ~name,
      () => {
        let _ =
          Oni_Extensions.NodeTask.run(
            ~scheduler=Scheduler.immediate,
            ~setup=Oni_Core.Setup.init(),
            "add-to-path.js",
          );
        ();
      },
    );

  let zoomEffect = (state: State.t, getCalculatedZoomValue, _) =>
    Isolinear.Effect.createWithDispatch(~name="window.zoom", dispatch => {
      let configuration = state.configuration;
      let currentZoomValue =
        Configuration.getValue(c => c.uiZoom, configuration);

      let calculatedZoomValue = getCalculatedZoomValue(currentZoomValue);
      let newZoomValue =
        Utility.IntEx.clamp(
          calculatedZoomValue,
          ~hi=Constants.maxZoomValue,
          ~lo=Constants.minZoomValue,
        );

      if (newZoomValue != currentZoomValue) {
        dispatch(
          ConfigurationSet({
            ...configuration,
            default: {
              ...configuration.default,
              uiZoom: newZoomValue,
            },
          }),
        );
      };
    });

  let commands = [
    ("system.addToPath", _ => togglePathEffect),
    ("system.removeFromPath", _ => togglePathEffect),
    ("view.closeEditor", state => closeEditorEffect(state)),
    ("view.splitVertical", state => splitEditorEffect(state, Vertical)),
    ("view.splitHorizontal", state => splitEditorEffect(state, Horizontal)),
    ("window.moveLeft", state => windowMoveEffect(state, Left)),
    ("window.moveRight", state => windowMoveEffect(state, Right)),
    ("window.moveUp", state => windowMoveEffect(state, Up)),
    ("window.moveDown", state => windowMoveEffect(state, Down)),
    (
      "workbench.action.zoomIn",
      state => zoomEffect(state, zoom => zoom +. Constants.zoomStep),
    ),
    (
      "workbench.action.zoomOut",
      state => zoomEffect(state, zoom => zoom -. Constants.zoomStep),
    ),
    (
      "workbench.action.zoomReset",
      state => zoomEffect(state, _zoom => Constants.defaultZoomValue),
    ),
  ];

  let commandMap =
    List.fold_left(
      (prev, curr) => {
        let (command, handler) = curr;
        StringMap.add(command, handler, prev);
      },
      StringMap.empty,
      commands,
    );

  let updater = (state: State.t, action) => {
    switch (action) {
    | EnableKeyDisplayer => (
        {...state, keyDisplayer: Some(KeyDisplayer.initial)},
        Isolinear.Effect.none,
      )

    | DisableKeyDisplayer => (
        {...state, keyDisplayer: None},
        Isolinear.Effect.none,
      )

    | Command(cmd) =>
      switch (StringMap.find_opt(cmd, commandMap)) {
      | Some(v) => (state, v(state, cmd))
      | None => (state, Isolinear.Effect.none)
      }
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
