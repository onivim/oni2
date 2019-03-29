/*
 * ExtensionClientHelper
 *
 * A module providing a helper API to make it easier to test
 * against the extension client API
 */

open Oni_Core;
open Oni_Extensions;

type filterFunction = (Yojson.Safe.json) => bool;
type waiterFunction = unit => unit;

type internalWaiter = {
    isActive: ref(bool),
    scope: string,
    method: string,
    filterFunction: filterFunction,
}

type extHostApi = {
    createWaiterForMessage: (string, string, filterFunction) => waiterFunction,
    send: unit => unit,
};

 let withExtensionClient = (
   f: (extHostApi) => unit
 ) => {
    let setup = Setup.init();

    let initialized = ref(false);
    let closed = ref(false);

    let onClosed = () => closed := true;
    let onInitialized = () => initialized := true;
    let extClient =
      ExtensionHostClient.start(~onInitialized, ~onClosed, setup);

    Oni_Core.Utility.waitForCondition(() => {
      ExtensionHostClient.pump(extClient);
      initialized^;
    });

    if (!initialized^) {
        failwith("extension host client did not initialize successfully");
    }

    let createWaiterForMessage = (scope, method, filterFunction) => {
        () => ();   
    };

    let send = () => ();

    let api: extHostApi = {
        createWaiterForMessage,
        send,
    }

    f(api);


    ExtensionHostClient.close(extClient);
    Oni_Core.Utility.waitForCondition(() => {
      ExtensionHostClient.pump(extClient);
      closed^;
    });
 };
