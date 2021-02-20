open Oni_Core;
open Utility;
open Exthost.Extension;

module ThemeLoaderSub =
  Isolinear.Sub.Make({
    type nonrec msg = result(
      (ColorTheme.variant,
      Textmate.ColorTheme.t,
      Oni_Syntax.TokenTheme.t,
    ), string);

    type nonrec params = Contributions.Theme.t;

    type state = unit;

    let name = "Feature_Theme.LoaderSub";
    let id = (params: Contributions.Theme.t) => params.path;

    let init = (~params, ~dispatch) => {
      let { uiTheme, path, _}: Contributions.Theme.t = params;
      let isDark = uiTheme == "vs-dark" || uiTheme == "hc-black";

      let loadResult: msg = path
      |> Textmate.Theme.from_file(~isDark)
      |> Result.map((theme) => {
           let colors = Textmate.Theme.getColors(theme);
           let isDark = Textmate.Theme.isDark(theme);

           let variant = isDark ? ColorTheme.Dark : ColorTheme.Light;
           let tokenColors =
             theme |> Textmate.Theme.getTokenColors |> Oni_Syntax.TokenTheme.create;

           (variant, colors, tokenColors);
         });

       let () = dispatch(loadResult);
    };

    let update = (~params as _, ~state, ~dispatch as _) => {
      state;
    };

    let dispose = (~params as _, ~state as _) => {
      ();
    };
  });


let sub = (theme) => ThemeLoaderSub.create(
  theme
);
