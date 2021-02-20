/*
 * ThemeStoreConnector.re
 *
 * This connector handles loading themes and tokenThemes.
 */

open Oni_Core;
open Oni_Model;
open Oni_Syntax;

module Log = (val Log.withNamespace("Oni2.Store.Theme"));

let start = () => {

  let loadThemeByIdEffect = (~extensions, themeId) => {
    Log.infof(m => m("Loading theme by id: %s", themeId));
    Isolinear.Effect.none;
    // let maybeTheme = Feature_Extensions.themeById(~id=themeId, extensions);

    // let errorEffect =
    //   switch (maybeTheme) {
    //   | Some(_) => Isolinear.Effect.none
    //   | None =>
    //     Feature_Notification.Effects.create(
    //       ~kind=Error,
    //       "Unable to find theme: " ++ themeId,
    //     )
    //     |> Isolinear.Effect.map(msg => Actions.Notification(msg))
    //   };

    // let loadThemeEffect =
    //   maybeTheme
    //   |> Utility.OptionEx.or_lazy(() => {
           // If we were unable to load, fall back to the default
    //        Feature_Extensions.themeById(
    //          ~id=Constants.defaultTheme,
    //          extensions,
    //        )
    //      })
    //   |> Option.map(
    //        ({uiTheme, path, _}: Exthost.Extension.Contributions.Theme.t) =>
    //        loadThemeByPathEffect(uiTheme, path)
    //      )
    //   |> Option.value(~default=Isolinear.Effect.none);

    // [errorEffect, loadThemeEffect] |> Isolinear.Effect.batch;
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

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Actions.ListFocusUp
    | Actions.ListFocusDown
    | Actions.ListFocus(_) =>
      switch (state.quickmenu) {
      | Some({variant: ThemesPicker(_), focused: Some(focused), items, _}) =>
        let focusedItem = items[focused];
        (
          {
            ...state,
            colorTheme: Feature_Theme.setTheme(~themeId=focusedItem.name, state.colorTheme)
          },
          Isolinear.Effect.none
        );
      | _ => (state, Isolinear.Effect.none)
      }

    | Actions.ThemeSelected(themeId) => (
        {
          ...state,
          colorTheme: Feature_Theme.setTheme(~themeId, state.colorTheme)
        },
        Isolinear.Effect.batch([
          persistThemeEffect(themeId),
        ]),
      )

    // TODO: Remove once wired up to syntax
    // | ThemeChanged(id) => (
    //     state,
    //     loadThemeByIdEffect(~extensions=state.extensions, id),
    //   )

    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
