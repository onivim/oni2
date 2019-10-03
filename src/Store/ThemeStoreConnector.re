/*
 * ThemeStoreConnector.re
 *
 * This connector handles loading themes and tokenThemes.
 */

open Oni_Core;
open Oni_Extensions;
open Oni_Model;
open Oni_Syntax;

let start = (themeInfo: ThemeInfo.t, setup: Setup.t) => {
  Log.info(ThemeInfo.show(themeInfo));

  let defaultThemePath =
    setup.bundledExtensionsPath ++ "/onedark-pro/themes/OneDark-Pro.json";

  let loadThemeByPathEffect = themePath =>
    Isolinear.Effect.createWithDispatch(
      ~name="theme.loadThemeByPath", dispatch =>
      Log.perf("theme.load", () => {
        let theme = Textmate.Theme.from_file(themePath);
        let colors = Textmate.Theme.getColors(theme);
        let isDark = Textmate.Theme.isDark(theme);
        let tokenColors = Textmate.Theme.getTokenColors(theme);
        let colors = Oni_Core.Theme.ofColorTheme(isDark, colors);
        /*let themeJson = Yojson.Safe.from_file(themePath);


          let colorsJson = Yojson.Safe.Util.member("colors", themeJson);
          let theme = Theme.of_yojson(colorsJson);*/

        // TODONOW: Convert received colors to token colors

        /*let tokenColorsJson =
            Yojson.Safe.Util.member("tokenColors", themeJson);
          let textMateTheme =
            Textmate.Theme.of_yojson(
              ~defaultBackground="#2F3440",
              ~defaultForeground="#DCDCDC",
              tokenColorsJson,
            );*/

        let tokenTheme = TokenTheme.create(tokenColors);

        dispatch(Actions.SetColorTheme(colors));
        dispatch(Actions.DarkModeSet(isDark));
        dispatch(Actions.SetTokenTheme(tokenTheme));
      })
    );

  let showThemeMenuEffect =
    Isolinear.Effect.createWithDispatch(~name="theme.showThemeMenu", dispatch => {
      let commands =
        ThemeInfo.getThemes(themeInfo)
        |> List.map((theme: ExtensionContributions.Theme.t) => {
             let ret: Actions.menuCommand = {
               category: Some("Theme"),
               name: theme.label,
               command: () => Actions.ThemeLoadByPath(theme.path),
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

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Actions.Init => (state, loadThemeByPathEffect(defaultThemePath))
    | Actions.ThemeShowMenu => (state, showThemeMenuEffect)
    | Actions.ThemeLoadByPath(themePath) => (
        state,
        loadThemeByPathEffect(themePath),
      )
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
