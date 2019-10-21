/*
 * TitleStoreConnector
 *
 * This implements an updater (reducer + side effects) for the window title
 */

module Core = Oni_Core;
module Model = Oni_Model;

module Actions = Model.Actions;

let start = (setTitle) => {

  let _lastTitle = ref("");

  let getTemplateVariables: Model.State.t => Core.StringMap.t(string) = (state) => {

    let initialValues = [("appName", "Onivim 2")];

    let initialValues = switch(Model.Selectors.getActiveBuffer(state)) {
    | None => initialValues
    | Some(buf) => 
      let fp = Model.Buffer.getFilePath(buf);
      let ret = switch (fp) {
      | None => initialValues;
      | Some(fp) => 
        let activeEditorShort = Filename.basename(fp);
        [("activeEditorShort", activeEditorShort), ...initialValues]
      };
      switch(Model.Buffer.isModified(buf)) {
      | false => ret
      | true => [("dirty", "*"), ...ret];
      }
    };

    let initialValues = switch(state.workspace) {
    | None => initialValues
    | Some(workspace) => [("rootName", workspace.rootName), ...initialValues]
    };

    initialValues
    |> List.to_seq
    |> Core.StringMap.of_seq;
  };

  let updateTitleEffect = (state) => 
    Isolinear.Effect.createWithDispatch(
    ~name="title.update", (dispatch) => {
  

    let templateVariables = getTemplateVariables(state);
    let titleTemplate = Core.Configuration.getValue((c) => c.windowTitle, state.configuration);
    
    let titleModel = Model.Title.ofString(titleTemplate);
    let title = Model.Title.toString(titleModel, templateVariables);
    print_endline ("!!! got title: " ++ title);

    if (!String.equal(_lastTitle^, title)) {
      print_endline("!!! TITLE: " ++ title);
      _lastTitle := title;
      setTitle(title);
    }
    
    
    });

  let updater = (state: Model.State.t, action: Actions.t) => {
    switch (action) {
    | Init => (state, updateTitleEffect(state))
    | BufferEnter(_) => (state, updateTitleEffect(state))
    | BufferSetModified(_) => (state, updateTitleEffect(state))
    // Catch directory changes
    | OpenExplorer(_) => (state, updateTitleEffect(state))
    | _ => (state, Isolinear.Effect.none)
    };
  };
  updater;
};
