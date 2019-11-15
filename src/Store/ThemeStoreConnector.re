/*
 * ThemeStoreConnector.re
 *
 * This connector handles loading themes and tokenThemes.
 */

open Oni_Core;
open Oni_Model;
open Oni_Syntax;

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

  let loadThemeByPath = (uiTheme, themePath, dispatch) => {
    Log.perf("theme.load", () => {
      let dark = uiTheme == "vs-dark" || uiTheme == "hc-black";
      let theme = Textmate.Theme.from_file(~isDark=dark, themePath);
      let colors = Textmate.Theme.getColors(theme);
      let isDark = Textmate.Theme.isDark(theme);
      let tokenColors = Textmate.Theme.getTokenColors(theme);
      let colors = Oni_Core.Theme.ofColorTheme(uiTheme, colors);
      let tokenTheme = TokenTheme.create(tokenColors);

      dispatch(Actions.SetColorTheme(colors));
      dispatch(Actions.DarkModeSet(isDark));
      dispatch(Actions.SetTokenTheme(tokenTheme));
    });
  };

  let loadThemeByName = (themeName, dispatch) => {
    let themeInfo = ThemeInfo.getThemeByName(themeInfo, themeName);

    switch (themeInfo) {
    | Some({uiTheme, path, _}) => loadThemeByPath(uiTheme, path, dispatch)
    | None =>
      dispatch(
        Actions.ShowNotification(
          Notification.create(
            ~notificationType=Actions.Error,
            ~title="Error",
            ~message="Unable to find theme: " ++ themeName,
            (),
          ),
        ),
      )
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

  let loadThemeByPathEffect = (uiTheme, themePath) =>
    Isolinear.Effect.createWithDispatch(
      ~name="theme.loadThemeByPath", dispatch =>
      loadThemeByPath(uiTheme, themePath, dispatch)
    );

  let loadThemeByNameEffect = name =>
    Isolinear.Effect.createWithDispatch(
      ~name="theme.loadThemeByName", dispatch =>
      loadThemeByName(name, dispatch)
    );

  let onChanged = (newTheme, dispatch) =>
    loadThemeByName(newTheme, dispatch);
  let withWatcher =
    configurationWatcher(c => c.workbenchColorTheme, onChanged);

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Actions.ListFocusUp
    | Actions.ListFocusDown
    | Actions.ListFocus(_) =>
      switch (state.menu) {
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

    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater |> withWatcher;
};
