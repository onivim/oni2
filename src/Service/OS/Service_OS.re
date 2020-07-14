open Oni_Core;
open Oni_Core.Utility;
module Log = (val Log.withNamespace("Oni2.Service.OS"));

let wrap = LuvEx.wrapPromise;

let bind = (fst, snd) => Lwt.bind(snd, fst);

module Internal = {
  let opendir = wrap(Luv.File.opendir);
  let closedir = wrap(Luv.File.closedir);
  let readdir = wrap(Luv.File.readdir);
};

module Api = {
  exception LuvException(Luv.Error.t);
  exception NotImplemented;

  let unlink = {
    let wrapped = wrap(Luv.File.unlink);

    path => {
      Log.infof(m => m("Luv.unlink: %s", path));
      wrapped(path);
    };
  };

  let rmdirNonRecursive = {
    let wrapped = wrap(Luv.File.rmdir);
    path => {
      Log.infof(m => m("Luv.rmdir: %s", path));
      wrapped(path);
    };
  };

  let stat = str => str |> wrap(Luv.File.stat);

  let readDir = path => {
    path
    |> Internal.opendir
    |> bind(dir => {
         Internal.readdir(dir) |> Lwt.map(items => (dir, items))
       })
    |> bind(((dir, results: array(Luv.File.Dirent.t))) => {
         Internal.closedir(dir) |> Lwt.map(() => results |> Array.to_list)
       });
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

  let rmdir = (~recursive=true, path) => {
    Log.tracef(m => m("rmdir called for path: %s", path));
    let rec loop = candidate => {
      Log.tracef(m => m("rmdir - recursing to: %s", path));
      candidate
      |> Internal.opendir
      |> bind(dir => {
           Lwt.bind(Internal.readdir(dir), dirents => {
             dirents
             |> Array.to_list
             |> List.map((dirent: Luv.File.Dirent.t) => {
                  let name = Rench.Path.join(candidate, dirent.name);
                  switch (dirent.kind) {
                  | `LINK
                  | `FILE => unlink(Rench.Path.join(candidate, dirent.name))
                  | `DIR => loop(Rench.Path.join(candidate, name))
                  | _ =>
                    Log.warnf(m =>
                      m("Unknown file type encountered: %s", name)
                    );
                    Lwt.return();
                  };
                })
             |> Lwt.join
             |> bind(_ => Internal.closedir(dir))
             |> bind(_ => rmdirNonRecursive(candidate))
           })
         });
    };
    if (recursive) {
      loop(path);
    } else {
      rmdirNonRecursive(path);
    };
  };

  let delete = (~recursive, path) => rmdir(~recursive, path);
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
