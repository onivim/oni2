open Oni_Core;
open Oni_Core_Test;
open Oni_Extensions;

open TestFramework;

describe("Extension Client", ({test, _}) =>
  test("gets initialized message", ({expect}) =>
    Helpers.repeat(() => {
      let setup = Setup.init();

      let initialized = ref(false);

      let onInitialized = () => initialized := true;
      let extClient = ExtensionHostClient.start(~onInitialized, setup);

      Oni_Core.Utility.waitForCondition(() => {
        ExtensionHostClient.pump(extClient);
        initialized^;
      });
      expect.bool(initialized^).toBe(true);
    })
  )
);
