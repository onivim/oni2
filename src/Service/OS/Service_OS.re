open Oni_Core;
module Log = (val Log.withNamespace("Oni2.Service.OS"));
module Imperative = {
  exception NotImplemented;

  let stat = path => {
    Luv.File.stat(path) |> Oni_Core.Utility.LuvEx.wrapPromise;
  };

  let readdir = _path => {
    Lwt.fail(NotImplemented);
  };

  let readFile = _path => {
    Lwt.fail(NotImplemented);
  };

  let writeFile = (_path, _bytes) => {
    Lwt.fail(NotImplemented);
  };

  let rename = (~source as _, ~target as _, ~overwrite as _) => {
    Lwt.fail(NotImplemented);
  };

  let copy = (~source as _, ~target as _, ~overwrite as _) => {
    Lwt.fail(NotImplemented);
  };

  let mkdir = _path => {
    Lwt.fail(NotImplemented);
  };

  let delete = (~recursive as _, ~useTrash as _, _path) => {
    Lwt.fail(NotImplemented);
  };
};

module Effect = {
  let openURL = url =>
    Isolinear.Effect.create(~name="os.openurl", () =>
      Revery.Native.Shell.openURL(url) |> (ignore: bool => unit)
    );

  let stat = (path, onResult) =>
    Isolinear.Effect.createWithDispatch(~name="os.stat", dispatch =>
      Unix.stat(path) |> onResult |> dispatch
    );

  let statMultiple = (paths, onResult) =>
    Isolinear.Effect.createWithDispatch(~name="os.statmultiple", dispatch =>
      List.iter(
        path => Unix.stat(path) |> onResult(path) |> dispatch,
        paths,
      )
    );
};
