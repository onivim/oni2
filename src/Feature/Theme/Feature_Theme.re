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
    Colors.Oni.defaults,
    Colors.Oni.Modal.defaults,
    Colors.Oni.Sneak.defaults,
    Colors.Panel.defaults,
    Colors.PanelTitle.defaults,
    Colors.PanelInput.defaults,
    Colors.ScrollbarSlider.defaults,
    Colors.Selection.defaults,
    Colors.SideBar.defaults,
    Colors.StatusBar.defaults,
    Colors.Tab.defaults,
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
};

let colors =
    (
      ~extensionDefaults as _=[], // TODO
      ~customizations=ColorTheme.Colors.empty, // TODO
      model,
    ) => {
  let {schema, theme} = model;

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
            (key, color, acc) =>
              color == ""
                ? acc : [(ColorTheme.key(key), Color.hex(color)), ...acc],
            colors,
            [],
          )
          |> ColorTheme.Colors.fromList,
      },
    }
  };
};
