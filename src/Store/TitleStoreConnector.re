/*
 * TitleStoreConnector
 *
 * This implements an updater (reducer + side effects) for the window title
 */

module Core = Oni_Core;
module Model = Oni_Model;

module Actions = Model.Actions;

let getDirectory = (fp: string): option(string) => {
  let dirs =
    Filename.dirname(fp) |> String.split_on_char(Filename.dir_sep.[0]);

  List.length(dirs) - 1 |> List.nth_opt(dirs);
};

let getTemplateVariables: Model.State.t => Core.StringMap.t(string) =
  state => {
    let initialValues = [("appName", "Onivim 2")];

    let initialValues =
      switch (Model.Selectors.getActiveBuffer(state)) {
      | None => initialValues
      | Some(buf) =>
        let fp = Core.Buffer.getFilePath(buf);
        let ret =
          switch (fp) {
          | None => initialValues
          | Some(fp) =>
            let activeEditorShort = Filename.basename(fp);
            let parentDir = getDirectory(fp);

            let initialValues = [
              ("activeEditorShort", activeEditorShort),
              ("activeEditorLong", fp),
              ...initialValues,
            ];

            switch (parentDir) {
            | None => initialValues
            | Some(dir) => [("activeFolderShort", dir), ...initialValues]
            };
          };
        switch (Core.Buffer.isModified(buf)) {
        | false => ret
        | true => [("dirty", "*"), ...ret]
        };
      };

    let initialValues =
      switch (state.workspace) {
      | None => initialValues
      | Some(workspace) => [
          ("rootName", workspace.rootName),
          ("rootPath", workspace.workingDirectory),
          ...initialValues,
        ]
      };

    initialValues |> List.to_seq |> Core.StringMap.of_seq;
  };

module Effects = {
  let updateTitle = state =>
    Isolinear.Effect.createWithDispatch(~name="title.update", dispatch => {
      let templateVariables = getTemplateVariables(state);
      let titleTemplate =
        Core.Configuration.getValue(c => c.windowTitle, state.configuration);

      let titleModel = Model.Title.ofString(titleTemplate);
      let title = Model.Title.toString(titleModel, templateVariables);

      dispatch(Actions.SetTitle(title));
    });
};

let start = setTitle => {
  let _lastTitle = ref("");

  let internalSetTitleEffect = title =>
    Isolinear.Effect.createWithDispatch(~name="title.set", _dispatch =>
      if (!String.equal(_lastTitle^, title)) {
        _lastTitle := title;
        setTitle(title);
      }
    );

  let updater = (state: Model.State.t, action: Actions.t) => {
    switch (action) {
    | Init => (state, Effects.updateTitle(state))
    | BufferEnter(_) => (state, Effects.updateTitle(state))
    | BufferSetModified(_) => (state, Effects.updateTitle(state))

    // TODO: This shouldn't exist, but needs to  be here because it deoends on
    // `setTitle` being passed in. It would however be ebtter to have a more
    // general mechanism, like "Effect actions" handled by an injected dependency
    // or have effects parameterized by an "environment" passed in along with
    // `dispatch`
    | SetTitle(title) => (state, internalSetTitleEffect(title))

    | _ => (state, Isolinear.Effect.none)
    };
  };
  updater;
};
