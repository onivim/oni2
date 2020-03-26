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

let start = (themeInfo: ThemeInfo.t) => {
  Log.info(ThemeInfo.show(themeInfo));

  let loadThemeByPathEffect = (uiTheme, themePath) =>
    Isolinear.Effect.createWithDispatch(
      ~name="theme.loadThemeByPath", dispatch => {
      Oni_Core.Log.perf("theme.load", () => {
        let dark = uiTheme == "vs-dark" || uiTheme == "hc-black";
        let theme = Textmate.Theme.from_file(~isDark=dark, themePath);
        let colors = Textmate.Theme.getColors(theme);
        let isDark = Textmate.Theme.isDark(theme);

        dispatch(
          Actions.Theme(
            Feature_Theme.TextmateThemeLoaded(
              isDark ? ColorTheme.Dark : ColorTheme.Light,
              colors,
            ),
          ),
        );

        let tokenColors = Textmate.Theme.getTokenColors(theme);
        let colors = Oni_Core.Theme.ofColorTheme(uiTheme, colors);
        let tokenTheme = TokenTheme.create(tokenColors);

        dispatch(Actions.SetColorTheme(colors));
        dispatch(Actions.DarkModeSet(isDark));
        dispatch(Actions.SetTokenTheme(tokenTheme));
      })
    });

  let loadThemeByNameEffect = themeName => {
    let themeInfo = ThemeInfo.getThemeByName(themeInfo, themeName);

    switch (themeInfo) {
    | Some({uiTheme, path, _}) => loadThemeByPathEffect(uiTheme, path)
    | None =>
      Feature_Notification.Effects.create(
        ~kind=Error,
        "Unable to find theme: " ++ themeName,
      )
      |> Isolinear.Effect.map(msg => Actions.Notification(msg))
    };
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
      | Some({variant: ThemesPicker, focused: Some(focused), items, _}) =>
        let focusedItem = items[focused];
        (state, loadThemeByNameEffect(focusedItem.name));
      | _ => (state, Isolinear.Effect.none)
      }

    | Actions.ThemeLoadByPath(uiTheme, themePath) => (
        state,
        loadThemeByPathEffect(uiTheme, themePath),
      )

    | Actions.ThemeLoadByName(name) => (
        state,
        Isolinear.Effect.batch([
          persistThemeEffect(name),
          loadThemeByNameEffect(name),
        ]),
      )

    | ThemeChanged(name) => (state, loadThemeByNameEffect(name))

    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater |> withWatcher;
};
