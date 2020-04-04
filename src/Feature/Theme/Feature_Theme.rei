open Oni_Core;

module Colors = GlobalColors;

type model;

let initial: list(list(ColorTheme.Schema.definition)) => model;

[@deriving show]
type msg =
  | TextmateThemeLoaded(ColorTheme.variant, [@opaque] Textmate.ColorTheme.t);

let update: (model, msg) => model;

let resolver:
  (
    ~extensionDefaults: list(ColorTheme.Defaults.t)=?,
    ~customizations: ColorTheme.Colors.t=?,
    model
  ) =>
  ColorTheme.resolver;
