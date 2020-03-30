open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Service.FileWatcher"));

[@deriving show({with_path: false})]
type event = {
  path: string,
  hasRenamed: bool,
  hasChanged: bool,
};

module WatchSubscription =
  Isolinear.Sub.Make({
    type state = {
      path: string,
      watcher: option(Luv.FS_event.t),
    };

    type nonrec msg = event;
    type nonrec params = string;

    let name = "FileWatcher";
    let id = Fun.id;

    let init = (~params as path, ~dispatch) => {
      Log.tracef(m => m("Starting file watcher for %s", path));

      switch (Luv.FS_event.init()) {
      | Ok(watcher) =>
        let onEvent = (
          fun
          | Ok((_file, events)) =>
            dispatch({
              path,
              hasRenamed: List.mem(`RENAME, events),
              hasChanged: List.mem(`CHANGE, events),
            })
          | Error(error) =>
            Log.errorf(m => m("'%s': %s", path, Luv.Error.strerror(error)))
        );

        Luv.FS_event.start(watcher, path, onEvent);

        {path, watcher: Some(watcher)};

      | Error(error) =>
        let message = Luv.Error.strerror(error);
        Log.errorf(m => m("init failed for '%s': %s", path, message));

        {path, watcher: None};
      };
    };

    let update = (~params as path, ~state, ~dispatch as _) =>
      if (path != state.path) {
        // Should be unreachable because the path is the id
        failwith(
          "unreachable",
        );
      } else {
        state;
      };

    let dispose = (~params as path, ~state) => {
      Log.tracef(m => m("Disposing file watcher for %s", path));

      switch (state.watcher) {
      | Some(watcher) => ignore(Luv__FS_event.stop(watcher): result(_))
      | None => ()
      };
    };
  });

let watch = (~path, ~onEvent) =>
  WatchSubscription.create(path)
  |> Isolinear.Sub.map(event => onEvent(event));
