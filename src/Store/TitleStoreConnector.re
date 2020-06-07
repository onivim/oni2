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

    let activeEditorShort =
      Option.bind(maybeBuffer, Buffer.getShortFriendlyName)
      |> withTag("activeEditorShort");

    let activeEditorMedium =
      Option.bind(maybeBuffer, buf =>
        Buffer.getMediumFriendlyName(
          ~workingDirectory=state.workspace.workingDirectory,
          buf,
        )
      )
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
           Some(Path.toRelative(~base=state.workspace.workingDirectory, fp))
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
      Some(("rootName", state.workspace.rootName)),
      Some(("rootPath", state.workspace.workingDirectory)),
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

let start = (setTitle, maximize, minimize, restore, close) => {
  let _lastTitle = ref("");

  let internalSetTitleEffect = title =>
    Isolinear.Effect.createWithDispatch(~name="title.set", _dispatch =>
      if (!String.equal(_lastTitle^, title)) {
        _lastTitle := title;

        setTitle(title);
      }
    );

  let internalDoubleClickEffect =
    Isolinear.Effect.create(~name="maximize", () =>
      switch (Revery.Environment.os) {
      | Mac =>
        let ic =
          Unix.open_process_in(
            "defaults read 'Apple Global Domain' AppleActionOnDoubleClick",
          );
        let operation = input_line(ic);
        switch (operation) {
        | "Maximize" => maximize()
        | "Minimize" => minimize()
        | _ => ()
        };
      | _ => ()
      }
    );

  let internalWindowCloseEffect =
    Isolinear.Effect.create(~name="window.close", () => close());
  let internalWindowMaximizeEffect =
    Isolinear.Effect.create(~name="window.maximize", () => maximize());
  let internalWindowMinimizeEffect =
    Isolinear.Effect.create(~name="window.minimize", () => minimize());
  let internalWindowRestoreEffect =
    Isolinear.Effect.create(~name="window.restore", () => restore());

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
    | TitleBar(Feature_TitleBar.TitleDoubleClicked) => (
        state,
        internalDoubleClickEffect,
      )
    | TitleBar(Feature_TitleBar.WindowCloseClicked) => (
        state,
        internalWindowCloseEffect,
      )
    | TitleBar(Feature_TitleBar.WindowMaximizeClicked) => (
        state,
        internalWindowMaximizeEffect,
      )
    | TitleBar(Feature_TitleBar.WindowRestoreClicked) => (
        state,
        internalWindowRestoreEffect,
      )
    | TitleBar(Feature_TitleBar.WindowMinimizeClicked) => (
        state,
        internalWindowMinimizeEffect,
      )

    | _ => (state, Isolinear.Effect.none)
    };
  };
  updater;
};
