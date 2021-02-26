open Oni_Core;
open Oni_Core.Utility;
module Log = (val Log.withNamespace("Oni2.Service.OS"));

let wrap = LuvEx.wrapPromise;

let bind = (fst, snd) => Lwt.bind(snd, fst);

module Internal = {
  let copyfile = (~overwrite=true, ~source, ~target) => {
    source |> wrap(Luv.File.copyfile(~excl=!overwrite, ~to_=target));
  };
  let openfile = (~flags, path) => flags |> wrap(Luv.File.open_(path));
  let closefile = wrap(Luv.File.close);
  let readfile = (~buffers, file) => buffers |> wrap(Luv.File.read(file));
  let opendir = wrap(Luv.File.opendir);
  let closedir = wrap(Luv.File.closedir);
  let readdir = wrap(Luv.File.readdir);
};

module Api = {
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

  let stat = path => {
    Log.infof(m => m("Luv.stat: %s", path));
    path
    |> wrap(Luv.File.stat)
    |> LwtEx.tap(_ => Log.infof(m => m("Stat completed: %s", path)));
  };

  let readdir = path => {
    Log.infof(m => m("readdir called for: %s", path));
    path
    |> Internal.opendir
    |> bind(dir => {
         Internal.readdir(dir) |> Lwt.map(items => (dir, items))
       })
    |> bind(((dir, results: array(Luv.File.Dirent.t))) => {
         Log.infof(m =>
           m("readdir returned %d items", Array.length(results))
         );
         Internal.closedir(dir) |> Lwt.map(() => results |> Array.to_list);
       });
  };

  let rec fold =
          (
            ~includeFiles,
            ~excludeDirectory,
            ~initial,
            accumulateFn: ('a, string) => 'a,
            rootPath,
          ) => {
    readdir(rootPath)
    |> LwtEx.flatMap(entries => {
         entries
         |> List.fold_left(
              (accPromise, {kind, name}: Luv.File.Dirent.t) => {
                let fullPath = Rench.Path.join(rootPath, name);
                if (kind == `FILE && includeFiles(fullPath)) {
                  let promise: Lwt.t('a) =
                    accPromise
                    |> LwtEx.flatMap(acc => {
                         Lwt.return(accumulateFn(acc, fullPath))
                       });
                  promise;
                } else if (kind == `DIR && !excludeDirectory(fullPath)) {
                  let promise: Lwt.t('a) =
                    accPromise
                    |> LwtEx.flatMap(acc => {
                         fold(
                           ~includeFiles,
                           ~excludeDirectory,
                           ~initial=acc,
                           accumulateFn,
                           fullPath,
                         )
                       });
                  promise;
                } else {
                  accPromise;
                };
              },
              Lwt.return(initial),
            )
       });
  };

  let glob = (~includeFiles=?, ~excludeDirectories=?, path) => {
    let includeFilesFn =
      includeFiles
      |> Option.map(filesGlobStr => {
           let regex =
             Re.Glob.glob(~expand_braces=true, filesGlobStr) |> Re.compile;
           p => Re.execp(regex, p);
         })
      |> Option.value(~default=_ => true);
    let excludeDirectoryFn =
      excludeDirectories
      |> Option.map(excludeDirectoryStr => {
           let regex =
             Re.Glob.glob(~expand_braces=true, excludeDirectoryStr)
             |> Re.compile;
           p => Re.execp(regex, p);
         })
      |> Option.value(~default=_ => false);

    fold(
      ~includeFiles=includeFilesFn,
      ~excludeDirectory=excludeDirectoryFn,
      ~initial=[],
      (acc, curr) => [curr, ...acc],
      path,
    );
  };

  let readFile = (~chunkSize=4096, path) => {
    let rec loop = (acc, file) => {
      let buffer = Luv.Buffer.create(chunkSize);
      Internal.readfile(~buffers=[buffer], file)
      |> bind((size: Unsigned.Size_t.t) => {
           let size = Unsigned.Size_t.to_int(size);

           if (size == 0) {
             Lwt.return(acc);
           } else {
             let bufferSub =
               if (size < chunkSize) {
                 Luv.Buffer.sub(buffer, ~offset=0, ~length=size);
               } else {
                 buffer;
               };
             loop([bufferSub, ...acc], file);
           };
         });
    };
    path
    |> Internal.openfile(~flags=[`RDONLY])
    |> bind(file => {
         loop([], file)
         |> bind((acc: list(Luv.Buffer.t)) => {
              file |> Internal.closefile |> Lwt.map(_ => acc)
            })
       })
    |> Lwt.map((buffers: list(Luv.Buffer.t)) => {
         LuvEx.Buffer.toBytesRev(buffers)
       });
  };

  let writeFile = (~contents, path) => {
    let buffer = Luv.Buffer.from_bytes(contents);
    path
    |> Internal.openfile(~flags=[`CREAT, `WRONLY])
    |> bind(file => {
         [buffer] |> wrap(Luv.File.write(file)) |> Lwt.map(_ => file)
       })
    |> bind(Internal.closefile);
  };

  let copy = (~source, ~target, ~overwrite) => {
    Internal.copyfile(~source, ~target, ~overwrite);
  };

  let rename = (~source, ~target, ~overwrite) => {
    copy(~source, ~target, ~overwrite) |> bind(() => unlink(source));
  };
  let mkdir = path => {
    path |> wrap(Luv.File.mkdir);
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
             |> List.fold_left(
                  (acc, dirent: Luv.File.Dirent.t) => {
                    acc
                    |> LwtEx.flatMap(_ => {
                         let name = Rench.Path.join(candidate, dirent.name);
                         switch (dirent.kind) {
                         | `LINK
                         | `FILE =>
                           unlink(Rench.Path.join(candidate, dirent.name))
                         | `DIR => loop(Rench.Path.join(candidate, name))
                         | _ =>
                           Log.warnf(m =>
                             m("Unknown file type encountered: %s", name)
                           );
                           Lwt.return();
                         };
                       })
                  },
                  Lwt.return(),
                )
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

  let mktempdir = (~prefix="temp-", ()) => {
    let rootTempPath = Filename.get_temp_dir_name();
    let tempFolderTemplate =
      Rench.Path.join(rootTempPath, Printf.sprintf("%sXXXXXX", prefix));
    tempFolderTemplate |> wrap(Luv.File.mkdtemp);
  };

  let delete = (~recursive, path) => rmdir(~recursive, path);

  let openURL = url =>
    if (Oni_Core.TrustedDomains.isUrlAllowed(url)) {
      Revery.Native.Shell.openURL(url);
    } else {
      Log.warnf(m =>
        m("URL %s was not opened because it was not trusted.", url)
      );
      false;
    };
};

// EFFECTS

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

  module Dialog = {
    let openFolder = (~initialDirectory=?, toMsg) => {
      Isolinear.Effect.createWithDispatch(
        ~name="os.dialog.openFolder", dispatch => {
        let maybeFolders =
          Revery_Native.Dialog.openFiles(
            ~startDirectory=?initialDirectory,
            ~canChooseDirectories=true,
            ~canChooseFiles=false,
            ~title="Open Folder",
            (),
          );

        let selectedFolders = maybeFolders |> Option.value(~default=[||]);

        if (Array.length(selectedFolders) > 0) {
          selectedFolders[0]
          |> FpExp.absoluteCurrentPlatform
          |> toMsg
          |> dispatch;
        } else {
          None |> toMsg |> dispatch;
        };
      });
    };
  };
};

// SUBSCRIPTIONS

module Sub = {
  type dirParams = {
    id: string,
    cwd: string,
  };
  module DirSub =
    Isolinear.Sub.Make({
      type nonrec msg = result(list(Luv.File.Dirent.t), string);

      type nonrec params = dirParams;

      type state = unit;

      let name = "Service_OS.Sub.dir";
      let id = ({id, cwd}) => id ++ cwd;

      let init = (~params, ~dispatch) => {
        let promise = Api.readdir(params.cwd);

        Lwt.on_success(promise, dirItems => dispatch(Ok(dirItems)));

        Lwt.on_failure(promise, exn =>
          dispatch(Error(Printexc.to_string(exn)))
        );

        ();
      };

      let update = (~params as _, ~state, ~dispatch as _) => {
        state;
      };

      let dispose = (~params as _, ~state as _) => {
        ();
      };
    });
  let dir = (~uniqueId, ~toMsg, path) => {
    DirSub.create({id: uniqueId, cwd: path}) |> Isolinear.Sub.map(toMsg);
  };
};
