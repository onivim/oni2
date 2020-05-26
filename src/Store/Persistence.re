open Oni_Core;

module Global = {
  module Schema = {
    open Oni_Model.State;
    open Persistence.Schema;

    let version =
      define("version", string, BuildInfo.commitId, _ => BuildInfo.commitId);
    let workspace =
      define("workspace", option(string), None, state =>
        Some(state.workspace.workingDirectory)
      );
  };

  open Persistence.Store;

  let store =
    instantiate("global", () => Schema.[entry(version), entry(workspace)]);

  let version = () => get(Schema.version, store);
  let workspace = () => get(Schema.workspace, store);

  let persist = state => persist(state, store);
};

module Workspace = {
  module Schema = {
    open Revery;
    open Persistence.Schema;

    let windowX =
      define("windowX", option(int), None, ((_state, window)) =>
        Some(Window.getPosition(window) |> fst)
      );
    let windowY =
      define("windowY", option(int), None, ((_state, window)) =>
        Some(Window.getPosition(window) |> snd)
      );
    let windowWidth =
      define("windowWidth", int, 800, ((_state, window)) =>
        Window.getSize(window).width
      );
    let windowHeight =
      define("windowHeight", int, 600, ((_state, window)) =>
        Window.getSize(window).height
      );
    let windowMaximized =
      define("windowMaximized", bool, false, ((_state, window)) =>
        Window.isMaximized(window)
      );
  };

  type state = (Oni_Model.State.t, Revery.Window.t);

  include Persistence.Store;

  let instantiate = path =>
    instantiate(path, () =>
      Schema.[
        entry(windowX),
        entry(windowY),
        entry(windowWidth),
        entry(windowHeight),
        entry(windowMaximized),
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

  let windowX = store => get(Schema.windowX, store);
  let windowY = store => get(Schema.windowY, store);
  let windowWidth = store => get(Schema.windowWidth, store);
  let windowHeight = store => get(Schema.windowHeight, store);
  let windowMaximized = store => get(Schema.windowMaximized, store);
};
