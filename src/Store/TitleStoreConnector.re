/*
 * TitleStoreConnector
 *
 * This implements an updater (reducer + side effects) for the window title
 */
open Oni_Core;
open Oni_Model;
open Utility;

let withTag = (tag: string, value: option(string)) =>
  Option.map(v => (tag, v), value);

let getTemplateVariables: State.t => StringMap.t(string) =
  state => {
    let maybeBuffer = Selectors.getActiveBuffer(state);
    let maybeFilePath = Option.bind(maybeBuffer, Buffer.getFilePath);

    let appName = Option.some("Onivim 2") |> withTag("appName");

    let dirty =
      Option.map(Buffer.isModified, maybeBuffer)
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
      Option.bind(maybeBuffer, Buffer.getShortFriendlyName)
      |> withTag("activeEditorShort");
    let activeEditorMedium =
      Option.bind(maybeBuffer, buf => {
        Option.bind(rootPath, ((_, nestedRootPath)) => {
          Buffer.getMediumFriendlyName(~workingDirectory=nestedRootPath, buf)
        })
      })
      |> withTag("activeEditorMedium");

    let activeEditorLong =
      Option.bind(maybeBuffer, Buffer.getLongFriendlyName)
      |> withTag("activeEditorLong");

    let activeFolderShort =
      Option.(
        maybeFilePath |> map(Filename.dirname) |> map(Filename.basename)
      )
      |> withTag("activeFolderShort");
    let activeFolderMedium =
      maybeFilePath
      |> Option.map(Filename.dirname)
      |> OptionEx.flatMap(fp =>
           switch (rootPath) {
           | Some((_, base)) => Some(Path.toRelative(~base, fp))
           | _ => None
           }
         )
      |> withTag("activeFolderMedium");
    let activeFolderLong =
      maybeFilePath
      |> Option.map(Filename.dirname)
      |> withTag("activeFolderLong");

    [
      appName,
      dirty,
      activeEditorShort,
      activeEditorMedium,
      activeEditorLong,
      activeFolderShort,
      activeFolderMedium,
      activeFolderLong,
      rootName,
      rootPath,
    ]
    |> OptionEx.values
    |> List.to_seq
    |> StringMap.of_seq;
  };

module Effects = {
  let updateTitle = state =>
    Isolinear.Effect.createWithDispatch(~name="title.update", dispatch => {
      let templateVariables = getTemplateVariables(state);
      let titleTemplate =
        Configuration.getValue(c => c.windowTitle, state.configuration);

      let titleModel = Title.ofString(titleTemplate);
      let title = Title.toString(titleModel, templateVariables);

      dispatch(Actions.SetTitle(title));
    });
};

let start = (setTitle, maximize) => {
  let _lastTitle = ref("");

  let internalSetTitleEffect = title =>
    Isolinear.Effect.createWithDispatch(~name="title.set", _dispatch =>
      if (!String.equal(_lastTitle^, title)) {
        _lastTitle := title;

        setTitle(title);
      }
    );

  let internalMaximizeEffect = () => Isolinear.Effect.createWithDispatch(~name="maximize", _dispatch => maximize());

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Init => (state, Effects.updateTitle(state))
    | BufferEnter(_) => (state, Effects.updateTitle(state))
    | BufferSetModified(_) => (state, Effects.updateTitle(state))

    // TODO: This shouldn't exist, but needs to  be here because it depends on
    // `setTitle` being passed in. It would however be better to have a more
    // general mechanism, like "Effect actions" handled by an injected dependency
    // or have effects parameterized by an "environment" passed in along with
    // `dispatch`
    | SetTitle(title) => (
        {...state, windowTitle: title},
        internalSetTitleEffect(title),
      )
    | Maximize => (state, internalMaximizeEffect())

    | _ => (state, Isolinear.Effect.none)
    };
  };
  updater;
};
