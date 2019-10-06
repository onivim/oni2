/*
 * ThemeStoreConnector.re
 *
 * This connector handles loading themes and tokenThemes.
 */

open Oni_Core;
open Oni_Extensions;
open Oni_Model;
open Oni_Syntax;

let configurationWatcher = (configurationSelector: ConfigurationValues.t => 'a, f) => {
  let oldValue: ref(option('a)) = ref(None);
  let configurationWatchEffect = (newValue: 'a) =>
    Isolinear.Effect.createWithDispatch(~name="configuration watcher", (dispatch) => {
      
      switch (oldValue^) {
      // If a === b, nothing changed - no need to call callback.
      | Some(a) when a == newValue => ();
      // Something changed, or this is the first time we're running.
      | _ =>
        f(newValue, dispatch);
        oldValue := Some(newValue);
        ();
      }
    });

  let newUpdater = (oldUpdater) => (state, action) => {
  switch (action) {
  | Actions.ConfigurationSet(config) => (state, configurationWatchEffect(
    Configuration.getValue(configurationSelector, config)));
  | _ => oldUpdater(state, action);
  }
  };

  newUpdater;
};

let start = (themeInfo: ThemeInfo.t, setup: Setup.t) => {
  Log.info(ThemeInfo.show(themeInfo));

/*  let defaultThemePath =
    setup.bundledExtensionsPath ++ "/onedark-pro/themes/OneDark-Pro.json";*/

  let loadThemeByPath = (uiTheme, themePath, dispatch) => {
      Log.perf("theme.load", () => {
        let theme = Textmate.Theme.from_file(themePath);
        let colors = Textmate.Theme.getColors(theme);
        let isDark = Textmate.Theme.isDark(theme);
        let tokenColors = Textmate.Theme.getTokenColors(theme);
        let colors = Oni_Core.Theme.ofColorTheme(uiTheme, colors);
        let tokenTheme = TokenTheme.create(tokenColors);

        dispatch(Actions.SetColorTheme(colors));
        dispatch(Actions.DarkModeSet(isDark));
        dispatch(Actions.SetTokenTheme(tokenTheme));
      })
  };

  let loadThemeByName = (themeName, dispatch) => {
    let themeInfo = ThemeInfo.getThemeByName(themeInfo, themeName);

    switch (themeInfo) {
    | Some({uiTheme, path, _}) => loadThemeByPath(uiTheme, path, dispatch);
    | None => dispatch(Actions.ShowNotification(Notification.create(~notificationType=Actions.Error,
      ~title="Error",
      ~message="Unable to find theme: " ++ themeName,
      ())));
    }
  };

  let loadThemeByPathEffect = (uiTheme, themePath) =>
    Isolinear.Effect.createWithDispatch(
      ~name="theme.loadThemeByPath", dispatch => loadThemeByPath(uiTheme, themePath, dispatch)
    );

  let showThemeMenuEffect =
    Isolinear.Effect.createWithDispatch(~name="theme.showThemeMenu", dispatch => {
      let commands =
        ThemeInfo.getThemes(themeInfo)
        |> List.map((theme: ExtensionContributions.Theme.t) => {
             let ret: Actions.menuCommand = {
               category: Some("Theme"),
               name: theme.label,
               command: () =>
                 Actions.ThemeLoadByPath(theme.uiTheme, theme.path),
               icon: None,
             };
             ret;
           });

      let create = (setItems, _, _) => {
        setItems(commands);
        () => ();
      };

      dispatch(Actions.MenuOpen(create));
    });

let onChanged = (newTheme, dispatch) => loadThemeByName(newTheme, dispatch);
let withWatcher = configurationWatcher(c => c.workbenchColorTheme , onChanged);

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
/*    | Actions.Init => (
        state,
        loadThemeByPathEffect("vs-dark", defaultThemePath),
      )*/
    | Actions.ThemeShowMenu => (state, showThemeMenuEffect)
    | Actions.ThemeLoadByPath(uiTheme, themePath) => (
        state,
        loadThemeByPathEffect(uiTheme, themePath),
      )
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater |> withWatcher;
};
