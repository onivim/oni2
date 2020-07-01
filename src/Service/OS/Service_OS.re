open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Service.OS"));

module Api = {
  exception LuvException(Luv.Error.t);

  let wrap = (f, arg) => {
    let (promise, resolver) = Lwt.task();

    f(arg, result => {
      switch (result) {
      | Ok(v) => Lwt.wakeup(resolver, v)
      | Error(err) => Lwt.wakeup_exn(resolver, LuvException(err))
      }
    });
    promise;
  };

  let opendir = wrap(Luv.File.opendir);
  let closedir = wrap(Luv.File.closedir);
  let readdir = wrap(Luv.File.readdir);
  let unlink = {
    let wrapped = wrap(Luv.File.unlink);

    path => {
      Log.infof(m => m("Luv.unlink: %s", path));
      wrapped(path);
    }
  };

  let rmdirNonRecursive = {
   let wrapped = wrap(Luv.File.rmdir);
    path => {
      Log.infof(m => m("Luv.rmdir: %s", path));
      wrapped(path);
    }
   };

  let bind = (fst, snd) => Lwt.bind(snd, fst);

  let rmdir = (~recursive=true, path) => {
    Log.tracef(m => m("rmdir called for path: %s", path));
    let rec loop = candidate => {
      Log.tracef(m => m("rmdir - recursing to: %s", path));
      candidate
      |> opendir
      |> bind(dir => {
           Lwt.bind(readdir(dir), dirents => {
             dirents
             |> Array.to_list
             |> List.map((dirent: Luv.File.Dirent.t) => {
                  let name = Rench.Path.join(candidate, dirent.name);
                  switch (dirent.kind) {
                  | `LINK
                  | `FILE =>
                    unlink(Rench.Path.join(candidate, dirent.name));
                  | `DIR =>
                    loop(Rench.Path.join(candidate, name));
                  | _ =>
                    Log.warnf(m => m("Unknown file type encountered: %s", name));
                    Lwt.return();
                  };
                })
             |> Lwt.join
             |> bind(_ => closedir(dir))
             |> bind((_) => rmdirNonRecursive(candidate));
           })
         });
    };
    if (recursive) {
      loop(path);
    } else {
      rmdirNonRecursive(path);
    };
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
