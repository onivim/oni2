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

              let promise = Service_OS.Api.stat(FpExp.toString(changedPath));

              Lwt.on_success(promise, statResult => {
                dispatch({
                  watchedPath: params.path,
                  changedPath: FpExp.At.(params.path / file),
                  hasRenamed: List.mem(`RENAME, events),
                  hasChanged: List.mem(`CHANGE, events),
                  stat: Some(statResult),
                })
              });

              Lwt.on_failure(promise, _exn => {
                dispatch({
                  watchedPath: params.path,
                  changedPath: FpExp.At.(params.path / file),
                  hasRenamed: List.mem(`RENAME, events),
                  hasChanged: List.mem(`CHANGE, events),
                  stat: None,
                })
              });
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

let watch = (~key, ~path, ~onEvent) =>
  WatchSubscription.create({key, path})
  |> Isolinear.Sub.map(event => onEvent(event));
