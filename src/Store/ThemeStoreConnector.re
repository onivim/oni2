/*
 * ThemeStoreConnector.re
 *
 * This connector handles loading themes and tokenThemes.
 */

open Oni_Core;
open Oni_Model;
open Oni_Syntax;

module Log = (val Log.withNamespace("Oni2.Store.Theme"));

let configurationWatcher =
    (configurationSelector: ConfigurationValues.t => 'a, f) => {
  let oldValue: ref(option('a)) = ref(None);
  let configurationWatchEffect = (newValue: 'a) =>
    Isolinear.Effect.createWithDispatch(
      ~name="configuration watcher", dispatch => {
      switch (oldValue^) {
      // If a === b, nothing changed - no need to call callback.
      | Some(a) when a == newValue => ()
      // Something changed, or this is the first time we're running.
      | _ =>
        f(newValue, dispatch);
        oldValue := Some(newValue);
        ();
      }
    });

  let newUpdater = (oldUpdater, state, action) => {
    switch (action) {
    | Actions.ConfigurationSet(config) => (
        state,
        configurationWatchEffect(
          Configuration.getValue(configurationSelector, config),
        ),
      )
    | _ => oldUpdater(state, action)
    };
  };

  newUpdater;
};

let start = () => {
  let loadThemeByPathEffect = (uiTheme, themePath) =>
    Isolinear.Effect.createWithDispatch(
      ~name="theme.loadThemeByPath", dispatch => {
      Oni_Core.Log.perf("theme.load", () => {
        let dark = uiTheme == "vs-dark" || uiTheme == "hc-black";

        Log.infof(m => m("Loading theme: %s", themePath));

        themePath
        |> Textmate.Theme.from_file(~isDark=dark)
        |> Utility.ResultEx.tapError(err => {
             dispatch(Actions.ThemeLoadError(err))
           })
        |> Result.iter(theme => {
             let colors = Textmate.Theme.getColors(theme);
             let isDark = Textmate.Theme.isDark(theme);

             let tokenColors =
               theme |> Textmate.Theme.getTokenColors |> TokenTheme.create;
             dispatch(
               Actions.Theme(
                 Feature_Theme.TextmateThemeLoaded({
                   variant: isDark ? ColorTheme.Dark : ColorTheme.Light,
                   colors,
                   tokenColors,
                 }),
               ),
             );
           });
      })
    });

  let loadThemeByIdEffect = (~extensions, themeId) => {
    Log.infof(m => m("Loading theme by id: %s", themeId));
    let maybeTheme = Feature_Extensions.themeById(~id=themeId, extensions);

    let errorEffect =
      switch (maybeTheme) {
      | Some(_) => Isolinear.Effect.none
      | None =>
        Feature_Notification.Effects.create(
          ~kind=Error,
          "Unable to find theme: " ++ themeId,
        )
        |> Isolinear.Effect.map(msg => Actions.Notification(msg))
      };

    let loadThemeEffect =
      maybeTheme
      |> Utility.OptionEx.or_lazy(() => {
           // If we were unable to load, fall back to the default
           Feature_Extensions.themeById(
             ~id=Constants.defaultTheme,
             extensions,
           )
         })
      |> Option.map(
           ({uiTheme, path, _}: Exthost.Extension.Contributions.Theme.t) =>
           loadThemeByPathEffect(uiTheme, path)
         )
      |> Option.value(~default=Isolinear.Effect.none);

    [errorEffect, loadThemeEffect] |> Isolinear.Effect.batch;
  };

  let persistThemeEffect = name =>
    Isolinear.Effect.createWithDispatch(~name="theme.persistTheme", dispatch =>
      dispatch(
        Actions.ConfigurationTransform(
          "configuration.json",
          Oni_Core.ConfigurationTransformer.setField(
            "workbench.colorTheme",
            `String(name),
          ),
        ),
      )
    );

  let onChanged = (newTheme, dispatch) =>
    dispatch(Actions.ThemeChanged(newTheme));
  let withWatcher =
    configurationWatcher(c => c.workbenchColorTheme, onChanged);

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Actions.ListFocusUp
    | Actions.ListFocusDown
    | Actions.ListFocus(_) =>
      switch (state.quickmenu) {
      | Some({variant: ThemesPicker(_), focused: Some(focused), items, _}) =>
        let focusedItem = items[focused];
        (
          state,
          loadThemeByIdEffect(~extensions=state.extensions, focusedItem.name),
        );
      | _ => (state, Isolinear.Effect.none)
      }

    | Actions.ThemeLoadByPath(uiTheme, themePath) => (
        state,
        loadThemeByPathEffect(uiTheme, themePath),
      )

    | Actions.ThemeLoadById(id) => (
        state,
        Isolinear.Effect.batch([
          persistThemeEffect(id),
          loadThemeByIdEffect(~extensions=state.extensions, id),
        ]),
      )

    | ThemeChanged(id) => (
        state,
        loadThemeByIdEffect(~extensions=state.extensions, id),
      )

    | Actions.ThemeLoadError(errorMsg) => (
        state,
        Feature_Notification.Effects.create(~kind=Error, errorMsg)
        |> Isolinear.Effect.map(msg => Actions.Notification(msg)),
      )
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater |> withWatcher;
};
