/*
 * ExtensionClientHelper
 *
 * A module providing a helper API to make it easier to test
 * against the extension client API
 */

open Oni_Core;
open Oni_Extensions;

type filterFunction = Yojson.Safe.json => bool;
type waiterFunction = unit => unit;

type internalWaiter = {
  isActive: ref(bool),
  scope: string,
  method: string,
  filterFunction,
};

type extHostApi = {
  createWaiterForMessage: (string, string, filterFunction) => waiterFunction,
  send: Yojson.Safe.json => unit,
};

let withExtensionClient = (f: extHostApi => unit) => {
  let setup = Setup.init();

  let initialized = ref(false);
  let closed = ref(false);

  let onClosed = () => closed := true;
  let onInitialized = () => initialized := true;

  let extClient = ref(None);

  let start = () => {
    let v = ExtensionHostClient.start(~onInitialized, ~onClosed, setup);

    Oni_Core.Utility.waitForCondition(() => {
      ExtensionHostClient.pump(v);
      initialized^;
    });

    if (! initialized^) {
      failwith("extension host client did not initialize successfully");
    };

    extClient := v;
  };

  let createWaiterForMessage = (_scope, _method, _filterFunction, ()) => ();

  let send = json => {
    switch (extClient) {
    | None =>
      failwith("Extension client not started - call start() before send()")
    | Some(extClient) => ExtensionHostClient.send(extClient, json)
    };
  };

  let api: extHostApi = {createWaiterForMessage, send, start};

  f(api);

  switch (extClient) {
  | None => ()
  | Some(extClient) =>
    ExtensionHostClient.close(extClient);
    Oni_Core.Utility.waitForCondition(() => {
      ExtensionHostClient.pump(extClient);
      closed^;
    });
  };
};
