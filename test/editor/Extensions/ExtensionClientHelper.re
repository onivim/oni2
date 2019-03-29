/*
 * ExtensionClientHelper
 *
 * A module providing a helper API to make it easier to test
 * against the extension client API
 */

open Oni_Core;
open Oni_Extensions;

type filterFunction = list(Yojson.Safe.json) => bool;
type waiterFunction = unit => unit;

type internalWaiter = {
  filter: (string, string, list(Yojson.Safe.json)) => bool,
  isCompleted: ref(bool),
};

type extHostApi = {
  createWaiterForMessage: (string, string, filterFunction) => waiterFunction,
  send: Yojson.Safe.json => unit,
  start: unit => unit,
};

let withExtensionClient = (f: extHostApi => unit) => {
  let setup = Setup.init();

  let rootPath = Rench.Environment.getWorkingDirectory();
  let testExtensionsPath = Rench.Path.join(rootPath, "test/test_extensions");

  let extensions =
    ExtensionScanner.scan(testExtensionsPath)
    |> List.map(ext =>
         ExtensionHostInitData.ExtensionInfo.ofScannedExtension(ext)
       );

  let initData = ExtensionHostInitData.create(~extensions, ());

  let initialized = ref(false);
  let closed = ref(false);

  let onClosed = () => closed := true;
  let onInitialized = () => initialized := true;

  let extClient = ref(None);

  let _waiters: ref(list(internalWaiter)) = ref([]);

  let createWaiterForMessage = (scope, method, filterFunction) => {
    let isActive = ref(false);

    let filter = (inScope, inMethod, payload) => {
      isActive^
      && String.equal(scope, inScope)
      && String.equal(method, inMethod)
      && filterFunction(payload);
    };

    let internalWaiter = {filter, isCompleted: ref(false)};

    _waiters := [internalWaiter, ..._waiters^];

    () => {
      isActive := true;

      Oni_Core.Utility.waitForCondition(
        ~timeout=10.0,
        () => {
          switch (extClient^) {
          | Some(v) => ExtensionHostClient.pump(v)
          | None =>
            failwith("extension client must be initialized prior to waiting")
          };
          internalWaiter.isCompleted^;
        },
      );

      if (! internalWaiter.isCompleted^) {
        failwith(
          "waiter failed for scope|method: " ++ scope ++ " | " ++ method,
        );
      };
    };
  };

  let onMessage = (scope, method, payload) => {
    let waitersToComplete =
      List.filter(f => f.filter(scope, method, payload), _waiters^);
    List.iter(w => w.isCompleted := true, waitersToComplete);
    Ok(None);
  };

  let start = () => {
    let v =
      ExtensionHostClient.start(
        ~initData,
        ~onInitialized,
        ~onMessage,
        ~onClosed,
        setup,
      );

    Oni_Core.Utility.waitForCondition(() => {
      ExtensionHostClient.pump(v);
      initialized^;
    });

    if (! initialized^) {
      failwith("extension host client did not initialize successfully");
    };

    extClient := Some(v);
  };

  let send = json => {
    switch (extClient^) {
    | None =>
      failwith("Extension client not started - call start() before send()")
    | Some(extClient) => ExtensionHostClient.send(extClient, json)
    };
  };

  let api: extHostApi = {createWaiterForMessage, send, start};

  f(api);

  switch (extClient^) {
  | None => ()
  | Some(extClient) =>
    ExtensionHostClient.close(extClient);
    Oni_Core.Utility.waitForCondition(() => {
      ExtensionHostClient.pump(extClient);
      closed^;
    });
  };
};
