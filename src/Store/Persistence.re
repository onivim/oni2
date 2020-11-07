open Oni_Core;

module Global = {
  module Schema = {
    open Oni_Model.State;
    open Persistence.Schema;

    let extensionValues =
      Feature_Extensions.(
        define(
          "extensionValues", Persistence.codec, Persistence.initial, state =>
          state.extensions |> Persistence.get(~shared=true)
        )
      );
    let version =
      define("version", string, BuildInfo.commitId, _ => BuildInfo.commitId);
    let workspace =
      define("workspace", option(string), None, state =>
        Some(state.workspace.workingDirectory)
      );
    let licenseKey =
      define("licenseKey", option(string), None, state =>
        state.registration.licenseKey
      );
  };

  open Persistence.Store;

  let store =
    lazy(
      {
        instantiate("global", () =>
          Schema.[
            entry(version),
            entry(workspace),
            entry(extensionValues),
            entry(licenseKey),
          ]
        );
      }
    );

  let extensionValues = () => get(Schema.extensionValues, Lazy.force(store));
  let version = () => get(Schema.version, Lazy.force(store));
  let workspace = () => get(Schema.workspace, Lazy.force(store));
  let licenseKey = () => get(Schema.licenseKey, Lazy.force(store));

  let persist = state => persist(state, Lazy.force(store));
};

module Workspace = {
  module Schema = {
    open Revery;
    open Persistence.Schema;

    let windowX =
      define("windowX", option(int), None, ((_state, window))
        // TODO: We should check if window is minimized
        => Some(Window.getPosition(window) |> fst));
    let windowY =
      define("windowY", option(int), None, ((_state, window))
        // TODO: We should check if window is minimized
        => Some(Window.getPosition(window) |> snd));
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

    let extensionValues =
      Feature_Extensions.(
        define(
          "extensionValues",
          Persistence.codec,
          Persistence.initial,
          ((state: Oni_Model.State.t, _window)) =>
          state.extensions |> Persistence.get(~shared=false)
        )
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
        entry(extensionValues),
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
  let extensionValues = store => get(Schema.extensionValues, store);
};
