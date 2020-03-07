open Oni_Core;
open Revery;

module Log = (val Oni_Core.Log.withNamespace("Oni2.Feature.Theme"));

module Colors = GlobalColors;

type model = {
  defaults: ColorTheme.Defaults.t,
  theme: ColorTheme.t,
};

let defaults =
  [
    Colors.ActivityBar.defaults,
    Colors.Editor.defaults,
    Colors.EditorGroupHeader.defaults,
    Colors.List.defaults,
    Colors.Oni.defaults,
    Colors.SideBar.defaults,
    Colors.Tab.defaults,
    Colors.defaults,
    Colors.remaining,
  ]
  |> List.map(ColorTheme.Defaults.fromList)
  |> ColorTheme.Defaults.unionMany;

let initial = contributions => {
  defaults: ColorTheme.Defaults.unionMany([defaults, ...contributions]),
  theme: ColorTheme.{variant: Dark, colors: ColorTheme.Colors.empty},
};

let resolver =
    (
      ~extensionDefaults as _=[], // TODO
      ~customizations=ColorTheme.Colors.empty, // TODO
      model,
    ) => {
  let {defaults, theme} = model;

  let rec resolve = keyName => {
    let key = ColorTheme.key(keyName);

    let fallback = keyName => {
      Log.warnf(m => m("Fallback color used for: %s", keyName));
      Some(Revery.Colors.magenta);
    };

    switch (ColorTheme.Colors.get(key, customizations)) {
    | Some(color) => Some(color)
    | None =>
      switch (ColorTheme.Colors.get(key, theme.colors)) {
      | Some(color) => Some(color)
      | None =>
        switch (ColorTheme.Defaults.get(key, defaults)) {
        | Some((entry: ColorTheme.Defaults.entry)) =>
          let colorValue =
            switch (theme.variant) {
            | Light => entry.light
            | Dark => entry.dark
            | HighContrast => entry.hc
            };

          switch (colorValue) {
          | Constant(color) => Some(color)
          | Reference(refName) when refName == keyName => fallback(keyName) // prevent infinite loop
          | Reference(refName) => resolve(refName)
          | Computed(f) => f(resolve)
          | Unspecified => None
          };

        | None => fallback(keyName)
        }
      }
    };
  };

  {
    as _;
    pub tryColor = resolve;
    pub color = key =>
      resolve(key) |> Option.value(~default=Revery.Colors.transparentWhite)
  };
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
