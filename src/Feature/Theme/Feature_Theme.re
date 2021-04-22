open Oni_Core;
open Revery;

module Log = (val Oni_Core.Log.withNamespace("Oni2.Feature.Theme"));

module Colors = GlobalColors;

type theme = Exthost.Extension.Contributions.Theme.t;

type model = {
  schema: ColorTheme.Schema.t,
  theme: ColorTheme.t,
  tokenColors: Oni_Syntax.TokenTheme.t,
  selectedThemeId: option(string),
};

let variant = ({theme, _}) => theme.variant;

let defaults =
  [
    Colors.ActivityBar.defaults,
    Colors.ActivityBarBadge.defaults,
    Colors.Button.defaults,
    Colors.Dropdown.defaults,
    Colors.Editor.defaults,
    Colors.EditorError.defaults,
    Colors.EditorWarning.defaults,
    Colors.EditorInfo.defaults,
    Colors.EditorHint.defaults,
    Colors.EditorCursor.defaults,
    Colors.EditorGroupHeader.defaults,
    Colors.EditorGutter.defaults,
    Colors.EditorHoverWidget.defaults,
    Colors.EditorIndentGuide.defaults,
    Colors.EditorLineNumber.defaults,
    Colors.EditorOverviewRuler.defaults,
    Colors.EditorRuler.defaults,
    Colors.EditorSuggestWidget.defaults,
    Colors.EditorWhitespace.defaults,
    Colors.EditorWidget.defaults,
    Colors.Input.defaults,
    Colors.List.defaults,
    Colors.Menu.defaults,
    Colors.Minimap.defaults,
    Colors.MinimapSlider.defaults,
    Colors.MinimapGutter.defaults,
    Colors.Notifications.defaults,
    Colors.Oni.defaults,
    Colors.Oni.Modal.defaults,
    Colors.Oni.Sneak.defaults,
    Colors.Panel.defaults,
    Colors.PanelTitle.defaults,
    Colors.PanelInput.defaults,
    Colors.ScrollbarSlider.defaults,
    Colors.Selection.defaults,
    Colors.SideBar.defaults,
    Colors.SideBarSectionHeader.defaults,
    Colors.StatusBar.defaults,
    Colors.SymbolIcon.defaults,
    Colors.Tab.defaults,
    Colors.TextLink.defaults,
    Colors.TitleBar.defaults,
    Colors.defaults,
  ]
  |> List.map(ColorTheme.Schema.fromList)
  |> ColorTheme.Schema.unionMany;

let initial = contributions => {
  schema:
    ColorTheme.Schema.unionMany([
      defaults,
      ...List.map(ColorTheme.Schema.fromList, contributions),
    ]),
  theme: ColorTheme.{variant: Dark, colors: ColorTheme.Colors.empty},
  tokenColors: Oni_Syntax.TokenTheme.empty,
  selectedThemeId: None,
};

let tokenColors = ({tokenColors, _}) => tokenColors;

let colors =
    (
      ~extensionDefaults as _=[], // TODO
      ~customizations=ColorTheme.Colors.empty, // TODO
      model,
    ) => {
  let {schema, theme, _} = model;

  let rec resolve = key => {
    switch (ColorTheme.Colors.get(key, customizations)) {
    | Some(color) => Some(color)
    | None =>
      switch (ColorTheme.Colors.get(key, theme.colors)) {
      | Some(color) => Some(color)
      | None =>
        switch (ColorTheme.Schema.get(key, schema)) {
        | Some(definition) =>
          definition.defaults
          |> ColorTheme.Defaults.get(theme.variant)
          |> ColorTheme.Defaults.evaluate(resolve)

        | None => None
        }
      }
    };
  };

  let defaults =
    schema
    |> ColorTheme.Schema.toList
    |> List.map(
         ColorTheme.Schema.(
           definition => (
             definition.key,
             definition.defaults
             |> ColorTheme.Defaults.get(theme.variant)
             |> ColorTheme.Defaults.evaluate(resolve)
             |> Option.value(~default=Color.hex("#0000")),
           )
         ),
       )
    |> ColorTheme.Colors.fromList;

  ColorTheme.Colors.unionMany([defaults, theme.colors, customizations]);
};

[@deriving show({with_path: false})]
type command =
  | SelectTheme;

[@deriving show({with_path: false})]
type msg =
  | Command(command)
  | MenuPreviewTheme({themeId: string})
  | MenuCommitTheme({themeId: string})
  | TextmateThemeLoaded({
      variant: ColorTheme.variant,
      colors: [@opaque] Textmate.ColorTheme.t,
      tokenColors: [@opaque] Oni_Syntax.TokenTheme.t,
    })
  | TextmateThemeLoadingError(string);

module Msg = {
  let openThemePicker = Command(SelectTheme);

  let vimColorSchemeSelected = (~themeId) =>
    MenuPreviewTheme({themeId: themeId});

  let menuPreviewTheme = (~themeId) => MenuPreviewTheme({themeId: themeId});

  let menuCommitTheme = (~themeId) => MenuCommitTheme({themeId: themeId});
};

let setTheme = (~themeId, model) => {
  ...model,
  selectedThemeId: Some(themeId),
};

type outmsg =
  | Nothing
  | ConfigurationTransform(ConfigurationTransformer.t)
  | OpenThemePicker(list(theme))
  | ThemeChanged(ColorTheme.Colors.t)
  | NotifyError(string);

let update = (model, msg) => {
  switch (msg) {
  | MenuCommitTheme({themeId}) =>
    let themeTransformer = name =>
      Oni_Core.ConfigurationTransformer.setField(
        "workbench.colorTheme",
        `String(name),
      );

    (
      model |> setTheme(~themeId),
      ConfigurationTransform(themeTransformer(themeId)),
    );

  | MenuPreviewTheme({themeId}) => (model |> setTheme(~themeId), Nothing)

  | TextmateThemeLoaded({variant, colors, tokenColors}) =>
    let colors =
      Textmate.ColorTheme.fold(
        (key, color, acc) =>
          color == ""
            ? acc : [(ColorTheme.key(key), Color.hex(color)), ...acc],
        colors,
        [],
      )
      |> ColorTheme.Colors.fromList;
    (
      {
        ...model,
        theme: {
          variant,
          colors,
        },
        tokenColors,
      },
      ThemeChanged(colors),
    );

  | TextmateThemeLoadingError(msg) => (
      {...model, selectedThemeId: Some(Constants.defaultTheme)},
      NotifyError(msg),
    )

  | Command(SelectTheme) => (model, OpenThemePicker([]))
  };
};

// SUBSCRIPTION

let sub = (~getThemeContribution, {selectedThemeId, _}) => {
  selectedThemeId
  |> Option.map(themeId => {
       ThemeLoader.sub(~themeId, ~getThemeContribution)
       |> Isolinear.Sub.map(
            fun
            | Ok((variant, colors, tokenColors)) =>
              TextmateThemeLoaded({variant, colors, tokenColors})
            | Error(msg) => {
                TextmateThemeLoadingError(msg);
              },
          )
     })
  |> Option.value(~default=Isolinear.Sub.none);
};

module Configuration = {
  open Oni_Core;
  open Config.Schema;

  let colorTheme =
    setting("workbench.colorTheme", string, ~default=Constants.defaultTheme);
};

let configurationChanged = (~resolver, model) => {
  {...model, selectedThemeId: Some(Configuration.colorTheme.get(resolver))};
};

module Commands = {
  open Feature_Commands.Schema;

  let selectTheme =
    define(
      ~category="Preferences",
      ~title="Theme Picker",
      "workbench.action.selectTheme",
      Command(SelectTheme),
    );
};

module Contributions = {
  let commands = [Commands.selectTheme];

  let configuration = Configuration.[colorTheme.spec];
};
