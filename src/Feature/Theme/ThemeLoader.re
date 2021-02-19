open Oni_Core;
open Exthost.Extension.Contributions;

module ThemeLoaderSub =
  Isolinear.Sub.Make({
    type nonrec msg = result(
      (ColorTheme.variant,
      Textmate.ColorTheme.t,
      Textmate.TokenTheme.t,
    ), string);

    type nonrec params = Theme.t;

    type state = unit;

    let name = "Feature_Theme.LoaderSub";
    let id = (params: Theme.t) => params.path;

    let init = (~params, ~dispatch) => {
      let { uiTheme, path, _}: Theme.t = params;
      let dark = uiTheme == "vs-dark" || uiTheme == "hc-black";
      dispatch(Error("No theme"));
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
