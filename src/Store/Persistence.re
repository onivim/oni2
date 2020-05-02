open Oni_Core;
open Persistence;

module Global = {
  open Oni_Model.State;
  open Schema;

  let version =
    define("version", string, BuildInfo.commitId, _ => BuildInfo.commitId);
  let workspace =
    define("workspace", option(string), None, state =>
      Some(state.workspace.workingDirectory)
    );

  let store =
    Store.instantiate("global", () =>
      [Store.entry(version), Store.entry(workspace)]
    );
};

module Workspace = {
  open Schema;

  type state = (Oni_Model.State.t, Revery.Window.t);

  let windowX =
    define("windowX", option(int), None, ((_state, window)) =>
      Some(Revery.Window.getPosition(window) |> fst)
    );
  let windowY =
    define("windowY", option(int), None, ((_state, window)) =>
      Some(Revery.Window.getPosition(window) |> snd)
    );
  let windowWidth =
    define("windowWidth", int, 800, ((_state, window)) =>
      Revery.Window.getRawSize(window).width
    );
  let windowHeight =
    define("windowHeight", int, 600, ((_state, window)) =>
      Revery.Window.getRawSize(window).height
    );
  let windowMaximized =
    define("windowMazimized", bool, false, ((_state, window)) =>
      Revery.Window.isMaximized(window)
    );

  let instantiate = path =>
    Store.instantiate(path, () =>
      [
        Store.entry(windowX),
        Store.entry(windowY),
        Store.entry(windowWidth),
        Store.entry(windowHeight),
        Store.entry(windowMaximized),
      ]
    );

  let storeFor = {
    let stores = Hashtbl.create(10);

    path =>
      switch (Hashtbl.find_opt(stores, path)) {
      | Some(store) => store
      | None =>
        let store = instantiate(path);
        Hashtbl.add(stores, path, store);
        store;
      };
  };
};
