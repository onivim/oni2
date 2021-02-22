/*
 * ThemeStoreConnector.re
 *
 * This connector handles loading themes and tokenThemes.
 */

open Oni_Core;
open Oni_Model;

module Log = (val Log.withNamespace("Oni2.Store.Theme"));

let start = () => {
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

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Actions.ListFocusUp
    | Actions.ListFocusDown
    | Actions.ListFocus(_) =>
      switch (state.quickmenu) {
      | Some({variant: ThemesPicker(_), focused: Some(focused), items, _}) =>
        let focusedItem = items[focused];
        (
          // While iterating through the menu, don't persist the theme
          // to configuration...
          {
            ...state,
            colorTheme:
              Feature_Theme.setTheme(
                ~themeId=focusedItem.name,
                state.colorTheme,
              ),
          },
          Isolinear.Effect.none,
        );
      | _ => (state, Isolinear.Effect.none)
      }

    | Actions.ThemeSelected(themeId) => (
        state,
        Isolinear.Effect.batch([
          // Persisting the theme will trigger a configuration changed,
          // which will cause the new theme to be picked up.
          persistThemeEffect(themeId),
        ]),
      )

    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
