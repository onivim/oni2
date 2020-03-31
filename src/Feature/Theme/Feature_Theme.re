open Oni_Core;
open Revery;

module Log = (val Oni_Core.Log.withNamespace("Oni2.Feature.Theme"));

module Colors = GlobalColors;

type model = {
  schema: ColorTheme.Schema.t,
  theme: ColorTheme.t,
};

let defaults =
  [
    Colors.ActivityBar.defaults,
    Colors.Dropdown.defaults,
    Colors.Editor.defaults,
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
    Colors.List.defaults,
    Colors.Menu.defaults,
    Colors.Oni.defaults,
    Colors.ScrollbarSlider.defaults,
    Colors.SideBar.defaults,
    Colors.StatusBar.defaults,
    Colors.Tab.defaults,
    Colors.defaults,
    Colors.remaining,
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
};

let resolver =
    (
      ~extensionDefaults as _=[], // TODO
      ~customizations=ColorTheme.Colors.empty, // TODO
      model,
    ) => {
  let {schema, theme} = model;

  let resolve = key => {
    switch (ColorTheme.Colors.get(key, customizations)) {
    | Some(color) => `Color(color)
    | None =>
      switch (ColorTheme.Colors.get(key, theme.colors)) {
      | Some(color) => `Color(color)
      | None =>
        switch (ColorTheme.Schema.get(key, schema)) {
        | Some(definition) =>
          `Default(
            ColorTheme.Defaults.get(theme.variant, definition.defaults),
          )

        | None => `NotRegistered
        }
      }
    };
  };

  resolve;
};

[@deriving show({with_path: false})]
type msg =
  | TextmateThemeLoaded(ColorTheme.variant, [@opaque] Textmate.ColorTheme.t);

let update = (model, msg) => {
  switch (msg) {
  | TextmateThemeLoaded(variant, colors) => {
      ...model,
      theme: {
        variant,
        colors:
          Textmate.ColorTheme.fold(
            (key, color, acc) => {
              color == "" ? acc : [(key, Color.hex(color)), ...acc]
            },
            colors,
            [],
          )
          |> ColorTheme.Colors.fromList,
      },
    }
  };
};
