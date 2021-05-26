open Oni_Core;
open Oni_Core.Utility;
module Log = (val Log.withNamespace("Oni2.Service.OS"));

let wrap = LuvEx.wrapPromise;

let bind = (fst, snd) => Lwt.bind(snd, fst);

module InnerApi = {
  let stat = path => {
    Log.infof(m => m("Luv.stat: %s", path));
    path
    |> wrap(Luv.File.stat)
    |> LwtEx.tap(_ => Log.infof(m => m("Stat completed: %s", path)));
  };

  let lstat = path => {
    Log.infof(m => m("Luv.lstat: %s", path));
    path
    |> wrap(Luv.File.lstat)
    |> LwtEx.tap(_ => Log.infof(m => m("Lstat completed: %s", path)));
  };
};

module Internal = {
  let copyfile = (~overwrite=true, ~source, target) => {
    source |> wrap(Luv.File.copyfile(~excl=!overwrite, ~to_=target));
  };
  let openfile = (~flags, path) => flags |> wrap(Luv.File.open_(path));
  let closefile = wrap(Luv.File.close);
  let readfile = (~buffers, file) => buffers |> wrap(Luv.File.read(file));
  let opendir = wrap(Luv.File.opendir);
  let closedir = wrap(Luv.File.closedir);
  let readdir = wrap(Luv.File.readdir);
};

module DirectoryEntry = {
  type kind = [ | `File | `Directory];

  type t = {
    kind,
    isSymbolicLink: bool,
    name: string,
    path: FpExp.t(FpExp.absolute),
  };

  let name = ({name, _}) => name;

  let path = ({path, _}) => path;

  let isSymbolicLink = ({isSymbolicLink, _}) => isSymbolicLink;

  let isFile = ({kind, _}) => kind == `File;

  let isDirectory = ({kind, _}) => kind == `Directory;

  let fromStat = (~name, path) => {
    let ofMode = (~isSymbolicLink, mode) => {
      let kind =
        if (Luv.File.Mode.test([`IFDIR], mode)) {
          `Directory;
        } else {
          `File;
        };

      Some({path, name, kind, isSymbolicLink});
    };

    let pathStr = FpExp.toString(path);

    Lwt.catch(
      () => {
        InnerApi.lstat(pathStr)
        |> LwtEx.flatMap((lstat: Luv.File.Stat.t) =>
             if (Luv.File.Mode.test([`IFLNK], lstat.mode)) {
               InnerApi.stat(pathStr)
               |> Lwt.map((stat: Luv.File.Stat.t) =>
                    ofMode(~isSymbolicLink=true, stat.mode)
                  );
             } else {
               Lwt.return(ofMode(~isSymbolicLink=false, lstat.mode));
             }
           )
      },
      exn => {
        Log.errorf(m =>
          m("Error stat'ing file: %s (%s)", pathStr, Printexc.to_string(exn))
        );
        Lwt.return(None);
      },
    );
  };

  let fromDirent =
      (~dirent: Luv.File.Dirent.t, path: FpExp.t(FpExp.absolute)) => {
    let name = dirent.name;
    switch (dirent.kind) {
    | `FILE =>
      Lwt.return(Some({name, kind: `File, isSymbolicLink: false, path}))
    | `DIR =>
      Lwt.return(Some({name, kind: `Directory, isSymbolicLink: false, path}))
    // Wasn't enough information from the direntry - could be `LINK, `UNKNOWN, etc -
    // fall back to using stat. We don't use stat by default for everything since the additional
    // I/O is slow on Windows
    | _ => fromStat(~name, path)
    };
  };
};

module Api = {
  include InnerApi;

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

  let readdir2 = path => {
    readdir(FpExp.toString(path))
    |> LwtEx.flatMap(dirItems => {
         let joiner = (acc, curr) => {
           [curr, ...acc];
         };
         let resolvedItems =
           dirItems
           |> List.map((dirent: Luv.File.Dirent.t) => {
                let fullPath = FpExp.At.(path / dirent.name);
                DirectoryEntry.fromDirent(~dirent, fullPath);
              });

         resolvedItems
         |> LwtEx.all(~initial=[], joiner)
         |> Lwt.map(OptionEx.values);
       });
  };

  let rec fold =
          (
            ~shouldContinue: 'a => bool,
            ~includeFiles,
            ~excludeDirectory,
            ~initial,
            accumulateFn: ('a, string) => 'a,
            rootPath,
          ) =>
    if (!shouldContinue(initial)) {
      Lwt.return(initial);
    } else {
      Lwt.catch(
        () => {
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
                                 ~shouldContinue,
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
             })
        },
        exn => {
          Log.warnf(m =>
            m(
              "Error while running Service_OS.fold on %s: %s",
              rootPath,
              Printexc.to_string(exn),
            )
          );
          Lwt.return(initial);
        },
      );
    };

  let glob = (~maxCount=?, ~includeFiles=?, ~excludeDirectories=?, path) => {
    let includeFilesFn =
      includeFiles
      |> Option.map(filesGlobStr => {
           let regex =
             Re.Glob.glob(~expand_braces=true, filesGlobStr) |> Re.compile;
           p => Re.execp(regex, Utility.Path.normalizeBackSlashes(p));
         })
      |> Option.value(~default=_ => true);
    let excludeDirectoryFn =
      excludeDirectories
      |> Option.map(excludeDirectoryStr => {
           let regex =
             Re.Glob.glob(~expand_braces=true, excludeDirectoryStr)
             |> Re.compile;
           p => Re.execp(regex, Utility.Path.normalizeBackSlashes(p));
         })
      |> Option.value(~default=_ => false);

    let shouldContinue =
      switch (maxCount) {
      | None => (_ => true)
      | Some(max) => (list => ListEx.boundedLength(~max, list) < max)
      };

    fold(
      ~shouldContinue,
      ~includeFiles=includeFilesFn,
      ~excludeDirectory=excludeDirectoryFn,
      ~initial=[],
      (acc, curr) => [curr, ...acc],
      path,
    )
    |> Lwt.map(items => {
         switch (maxCount) {
         | None => items
         | Some(count) => items |> ListEx.firstk(count)
         }
       });
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
    Internal.copyfile(~overwrite, ~source, target);
  };

  let rename = (~source, ~target, ~overwrite) => {
    copy(~source, ~target, ~overwrite) |> bind(() => unlink(source));
  };
  let mkdir = path => {
    Log.tracef(m => m("Calling mkdir for path: %s", path));
    let maybeStat = str => {
      Lwt.catch(
        () => stat(str) |> Lwt.map(stat => Some(stat)),
        _exn => Lwt.return(None),
      );
    };

    let isDirectory = (stat: Luv.File.Stat.t) => {
      Luv.File.Mode.test([`IFDIR], stat.mode);
    };

    let doMkdir = path => path |> wrap(Luv.File.mkdir);

    let attemptCheckOrCreateDirectory = path => {
      path
      |> maybeStat
      |> LwtEx.flatMap(maybeStatResult => {
           maybeStatResult
           |> Option.map(statResult =>
                if (isDirectory(statResult)) {
                  Lwt.return();
                } else {
                  doMkdir(path);
                }
              )
           |> OptionEx.value_or_lazy(() => doMkdir(path))
         });
    };

    let rec loop = attemptCount =>
      if (attemptCount >= 3) {
        attemptCheckOrCreateDirectory(path);
      } else {
        Lwt.catch(
          () => {attemptCheckOrCreateDirectory(path)},
          _exn => loop(attemptCount + 1),
        );
      };

    loop(0);
  };

  let mkdirp = (path: FpExp.t(FpExp.absolute)) => {
    let rec loop = path =>
      // Hit the root!
      if (FpExp.eq(path, FpExp.root) || FpExp.eq(path, FpExp.dirName(path))) {
        Lwt.return();
      } else {
        let parent = FpExp.dirName(path);
        let pathStr = FpExp.toString(path);

        let maybeStat = str => {
          Lwt.catch(
            () => stat(str) |> Lwt.map(stat => Some(stat)),
            _exn => Lwt.return(None),
          );
        };

        let isDirectory = (stat: Luv.File.Stat.t) => {
          Luv.File.Mode.test([`IFDIR], stat.mode);
        };

        parent
        |> loop
        |> LwtEx.flatMap(() => {
             // Does the directory already exist?
             maybeStat(pathStr)
           })
        |> LwtEx.flatMap(maybeStatResult => {
             switch (maybeStatResult) {
             | None => mkdir(pathStr)
             | Some(stat) when isDirectory(stat) => Lwt.return()
             | Some(_stat) =>
               Lwt.fail_with(
                 Printf.sprintf(
                   "mkdirp: Path %s exists but is not a directory",
                   pathStr,
                 ),
               )
             }
           });
      };

    loop(path);
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
        path =>
          try({
            let stat = Unix.stat(path);

            switch (stat.st_kind) {
            | Unix.S_REG =>
              onResult(~exists=true, ~isDirectory=false, path) |> dispatch
            | Unix.S_DIR =>
              onResult(~exists=true, ~isDirectory=true, path) |> dispatch
            | _ => ()
            };
          }) {
          | _exn =>
            onResult(~exists=false, ~isDirectory=false, path) |> dispatch
          },
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
    cwdPath: FpExp.t(FpExp.absolute),
    cwd: string,
  };
  module DirSub =
    Isolinear.Sub.Make({
      type nonrec msg = result(list(DirectoryEntry.t), string);

      type nonrec params = dirParams;

      type state = unit;

      let name = "Service_OS.Sub.dir";
      let id = ({id, cwd, _}) => id ++ cwd;

      let init = (~params, ~dispatch) => {
        let promise = params.cwdPath |> Api.readdir2;

        Lwt.on_success(promise, dirItems => {dispatch(Ok(dirItems))});

        Lwt.on_failure(promise, exn => {
          dispatch(Error(Printexc.to_string(exn)))
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
  let dir = (~uniqueId, ~toMsg, path) => {
    DirSub.create({id: uniqueId, cwdPath: path, cwd: FpExp.toString(path)})
    |> Isolinear.Sub.map(toMsg);
  };
};
