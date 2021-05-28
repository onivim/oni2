open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Service.FileWatcher"));

module Key =
  Oni_Core.UniqueId.Make({});

[@deriving show({with_path: false})]
type event = {
  watchedPath: [@opaque] FpExp.t(FpExp.absolute),
  changedPath: [@opaque] FpExp.t(FpExp.absolute),
  hasRenamed: bool,
  hasChanged: bool,
  stat: [@opaque] option(Luv.File.Stat.t),
};

type params = {
  watchChanges: bool,
  path: FpExp.t(FpExp.absolute),
  key: Key.t,
};

module WatchSubscription =
  Isolinear.Sub.Make({
    type state = {
      path: FpExp.t(FpExp.absolute),
      watcher: option(Luv.FS_event.t),
    };

    type nonrec msg = event;
    type nonrec params = params;

    let name = "FileWatcher";
    let id = params => {
      String.concat(
        ":",
        [params.key |> Key.toString, FpExp.toString(params.path)],
      );
    };

    let init = (~params: params, ~dispatch) => {
      Log.tracef(m =>
        m("Starting file watcher for %s", FpExp.toString(params.path))
      );

      let pathStr = FpExp.toString(params.path);
      switch (Luv.FS_event.init()) {
      | Ok(watcher) =>
        let onEvent = (
          fun
          | Ok((file, events)) => {
              let changedPath = FpExp.At.(params.path / file);
              let hasRenamed = List.mem(`RENAME, events);
              let hasChanged = List.mem(`CHANGE, events);

              let complete = maybeStatResult =>
                dispatch({
                  watchedPath: params.path,
                  changedPath: FpExp.At.(params.path / file),
                  hasRenamed,
                  hasChanged,
                  stat: maybeStatResult,
                });

              if (hasRenamed) {
                // PERF: #3373 - only stat if there was a rename (creation, unlink, etc)
                let promise =
                  Service_OS.Api.stat(FpExp.toString(changedPath));
                Lwt.on_success(promise, statResult => {
                  complete(Some(statResult))
                });

                Lwt.on_failure(promise, _exn => {complete(None)});
              } else if (params.watchChanges) {
                complete(None);
              } else {
                ();
              };
            }
          | Error(error) =>
            Log.errorf(m =>
              m("'%s': %s", pathStr, Luv.Error.strerror(error))
            )
        );

        Luv.FS_event.start(watcher, pathStr, onEvent);

        {path: params.path, watcher: Some(watcher)};

      | Error(error) =>
        let message = Luv.Error.strerror(error);
        Log.errorf(m => m("init failed for '%s': %s", pathStr, message));

        {path: params.path, watcher: None};
      };
    };

    let update = (~params as _, ~state, ~dispatch as _) =>
      // Since the path is the id, there's nothing to change
      state;

    let dispose = (~params: params, ~state) => {
      Log.tracef(m =>
        m("Disposing file watcher for %s", FpExp.toString(params.path))
      );

      switch (state.watcher) {
      | Some(watcher) => ignore(Luv__FS_event.stop(watcher): result(_))
      | None => ()
      };
    };
  });

let watch = (~watchChanges, ~key, ~path, ~onEvent) =>
  WatchSubscription.create({watchChanges, key, path})
  |> Isolinear.Sub.map(event => onEvent(event));
