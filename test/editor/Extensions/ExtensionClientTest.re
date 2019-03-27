open Oni_Core;
open Oni_Core_Test;
open Oni_Extensions;

open TestFramework;

describe("Extension Client", ({test, _}) => {
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

  test("doesn't die after a few seconds", ({expect}) => {
      let setup = Setup.init();

      let initialized = ref(false);
      let closed = ref(false);

      let onClosed = () => closed := true;
      let onInitialized = () => initialized := true;
      let extClient = ExtensionHostClient.start(~onInitialized, ~onClosed, setup);

      Oni_Core.Utility.waitForCondition(() => {
        ExtensionHostClient.pump(extClient);
        initialized^;
      });

      expect.bool(initialized^).toBe(true);

      /* The extension host process will die after a second if it doesn't see the parent PID */
      /* We'll sleep for two seconds to be safe */
      Unix.sleep(2);

      expect.bool(closed^).toBe(false);


        });
});
