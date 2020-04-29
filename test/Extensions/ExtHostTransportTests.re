open Oni_Core;
open Utility;
open Oni_Extensions;

open TestFramework;
open Exthost.Extension;

module Helpers = Oni_Core_Test.Helpers;

let initialConfiguration = Oni_Extensions.Configuration.empty;
let initData = ExtHostInitData.create();

describe("ExtHostTransport", ({test, _}) => {
  test("gets initialized message", ({expect, _}) =>
    Helpers.repeat(() => {
      let setup = Setup.init();
      let initialized = ref(false);
      let onInitialized = () => initialized := true;
      let extClient =
        ExtHostTransport.start(
          ~initData,
          ~initialConfiguration,
          ~onInitialized,
          setup,
        );
      ThreadEx.waitForCondition(() => {
        Revery.App.flushPendingCallbacks();
        initialized^;
      });
      expect.bool(initialized^).toBe(true);
      ExtHostTransport.close(extClient);
    })
  );
  test("doesn't die after a few seconds", ({expect, _}) => {
    let setup = Setup.init();
    let initialized = ref(false);
    let closed = ref(false);
    let onClosed = () => closed := true;
    let onInitialized = () => initialized := true;
    let _extClient =
      ExtHostTransport.start(
        ~initData,
        ~initialConfiguration,
        ~onInitialized,
        ~onClosed,
        setup,
      );
    ThreadEx.waitForCondition(() => {
      Revery.App.flushPendingCallbacks();
      initialized^;
    });
    expect.bool(initialized^).toBe(true);
    /* The extension host process will die after a second if it doesn't see the parent PID */
    /* We'll sleep for two seconds to be safe */
    Unix.sleep(2);
    expect.bool(closed^).toBe(false);
  });
  test("closes after close is called", ({expect, _}) => {
    let setup = Setup.init();
    let initialized = ref(false);
    let closed = ref(false);
    let onClosed = () => closed := true;
    let onInitialized = () => initialized := true;
    let extClient =
      ExtHostTransport.start(
        ~initData,
        ~initialConfiguration,
        ~onInitialized,
        ~onClosed,
        setup,
      );
    ThreadEx.waitForCondition(() => {
      Revery.App.flushPendingCallbacks();
      initialized^;
    });
    expect.bool(initialized^).toBe(true);
    ExtHostTransport.close(extClient);
    ThreadEx.waitForCondition(() => {closed^});
    expect.bool(closed^).toBe(false);
  });
  test("basic extension activation", _ => {
    let setup = Setup.init();
    let rootPath = Rench.Environment.getWorkingDirectory();
    let testExtensionsPath =
      Rench.Path.join(rootPath, "test/test_extensions");
    let extensions =
      Scanner.scan(~category=Development, testExtensionsPath)
      |> List.map(ext =>
           ExtHostInitData.ExtensionInfo.ofScannedExtension(ext)
         );
    let gotWillActivateMessage = ref(false);
    let gotDidActivateMessage = ref(false);
    let onMessage = (a, b, _c) => {
      switch (a, b) {
      | ("MainThreadExtensionService", "$onWillActivateExtension") =>
        gotWillActivateMessage := true
      | ("MainThreadExtensionService", "$onDidActivateExtension") =>
        gotDidActivateMessage := true
      | _ => ()
      };
      Ok(None);
    };
    let initData = ExtHostInitData.create(~extensions, ());
    let extClient =
      ExtHostTransport.start(
        ~initialConfiguration,
        ~initData,
        ~onMessage,
        setup,
      );
    ThreadEx.waitForCondition(() => {
      Revery.App.flushPendingCallbacks();
      gotWillActivateMessage^;
    });
    ThreadEx.waitForCondition(() => {
      Revery.App.flushPendingCallbacks();
      gotDidActivateMessage^;
    });
    ExtHostTransport.close(extClient);
  });
});
