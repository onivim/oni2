/*
 * TitleStoreConnector
 *
 * This implements an updater (reducer + side effects) for the window title
 */

module Core = Oni_Core;
module Model = Oni_Model;

module Actions = Model.Actions;
module Option = Core.Utility.Option;

let withTag = (tag: string, value: option(string)) =>
  Option.map(v => (tag, v), value);

let getTemplateVariables: Model.State.t => Core.StringMap.t(string) =
  state => {
    let buffer = Model.Selectors.getActiveBuffer(state);
    let filePath = Option.bind(Core.Buffer.getFilePath, buffer);

    let appName = Option.some("Onivim 2") |> withTag("appName");

    let dirty =
      Option.map(Core.Buffer.isModified, buffer)
      |> (
        fun
        | Some(true) => Some("*")
        | _ => None
      )
      |> withTag("dirty");

    let (rootName, rootPath) =
      switch (state.workspace) {
      | Some({rootName, workingDirectory}) => (
          Some(rootName) |> withTag("rootName"),
          Some(workingDirectory) |> withTag("rootPath"),
        )
      | None => (None, None)
      };

    let activeEditorShort =
      Option.map(Filename.basename, filePath) |> withTag("activeEditorShort");
    let activeEditorLong = filePath |> withTag("activeEditorLong");

    let activeFolderShort =
      Option.(filePath |> map(Filename.dirname) |> map(Filename.basename))
      |> withTag("activeFolderShort");

    [
      appName,
      dirty,
      activeEditorShort,
      activeEditorLong,
      activeFolderShort,
      rootName,
      rootPath,
    ]
    |> Option.values
    |> List.to_seq
    |> Core.StringMap.of_seq;
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
