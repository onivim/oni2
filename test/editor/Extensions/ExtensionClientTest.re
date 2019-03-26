open Oni_Core;
open Oni_Core_Test;
open Oni_Extensions;

open TestFramework;

describe("Extension Client", ({test, _}) => {
  test("receive init message", ({expect}) =>
    Helpers.repeat(() => {
      let setup = Setup.init();

      let gotReadyMessage = ref(false);

      let onClosed = () => ();
      let onMessage = (id, _) => {
          if (id === ExtensionHostClient.Protocol.MessageType.ready) {
            gotReadyMessage := true;
          }

          Ok(None);
      };

      let extClient =
        ExtensionHostClient.start(~onClosed, ~onMessage, setup);

      Oni_Core.Utility.waitForCondition(() => {
        ExtensionHostClient.pump(extClient);
        gotReadyMessage^;
      });
      expect.bool(gotReadyMessage^).toBe(true);

    }));
});
