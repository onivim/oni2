open Oni_Core;
open Exthost.Extension;

type params = {
  id: string,
  idToContribution: string => option(Exthost.Extension.Contributions.Theme.t),
};

module ThemeLoaderSub =
  Isolinear.Sub.Make({
    type nonrec msg =
      result(
        (ColorTheme.variant, Textmate.ColorTheme.t, Oni_Syntax.TokenTheme.t),
        string,
      );

    type nonrec params = params;

    type state = unit;

    let name = "Feature_Theme.LoaderSub";
    let id = params => params.id;

    let init = (~params, ~dispatch) => {
      let () =
        params.idToContribution(params.id)
        |> Option.to_result(~none="Unable to load theme: " ++ params.id)
        |> Utility.ResultEx.tapError(err => dispatch(Error(err)))
        |> Result.iter(contribution => {
             let {uiTheme, path, _}: Contributions.Theme.t = contribution;
             let isDark = uiTheme == "vs-dark" || uiTheme == "hc-black";

             let loadResult: msg =
               path
               |> Textmate.Theme.from_file(~isDark)
               |> Result.map(theme => {
                    let colors = Textmate.Theme.getColors(theme);
                    let isDark = Textmate.Theme.isDark(theme);

                    let variant = isDark ? ColorTheme.Dark : ColorTheme.Light;
                    let tokenColors =
                      theme
                      |> Textmate.Theme.getTokenColors
                      |> Oni_Syntax.TokenTheme.create;

                    (variant, colors, tokenColors);
                  });

             let () = dispatch(loadResult);
             ();
           });
      ();
    };

    let update = (~params as _, ~state, ~dispatch as _) => {
      state;
    };

    let dispose = (~params as _, ~state as _) => {
      ();
    };
  });

let sub = (~themeId, ~getThemeContribution) =>
  ThemeLoaderSub.create({
    id: themeId,
    idToContribution: getThemeContribution,
  });
