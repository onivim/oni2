/*
 TODO: Bring back for exthost work

 open Oni_Core;
 open Oni_Core_Test;
 open Oni_Extensions;

 open TestFramework;

 describe("ExtHostTransport", ({test, _}) => {
   test("gets initialized message", ({expect}) =>
     Helpers.repeat(() => {
       let setup = Setup.init();
       let initialized = ref(false);
       let onInitialized = () => initialized := true;
       let extClient = ExtHostTransport.start(~onInitialized, setup);
       Oni_Core.Utility.waitForCondition(() => {
         ExtHostTransport.pump(extClient);
         initialized^;
       });
       expect.bool(initialized^).toBe(true);
       ExtHostTransport.close(extClient);
     })
   );
   test("doesn't die after a few seconds", ({expect}) => {
     let setup = Setup.init();
     let initialized = ref(false);
     let closed = ref(false);
     let onClosed = () => closed := true;
     let onInitialized = () => initialized := true;
     let extClient = ExtHostTransport.start(~onInitialized, ~onClosed, setup);
     Oni_Core.Utility.waitForCondition(() => {
       ExtHostTransport.pump(extClient);
       initialized^;
     });
     expect.bool(initialized^).toBe(true);
     /* The extension host process will die after a second if it doesn't see the parent PID */
     /* We'll sleep for two seconds to be safe */
     Unix.sleep(2);
     expect.bool(closed^).toBe(false);
   });
   test("closes after close is called", ({expect}) => {
     let setup = Setup.init();
     let initialized = ref(false);
     let closed = ref(false);
     let onClosed = () => closed := true;
     let onInitialized = () => initialized := true;
     let extClient = ExtHostTransport.start(~onInitialized, ~onClosed, setup);
     Oni_Core.Utility.waitForCondition(() => {
       ExtHostTransport.pump(extClient);
       initialized^;
     });
     expect.bool(initialized^).toBe(true);
     ExtHostTransport.close(extClient);
     Oni_Core.Utility.waitForCondition(() => {
       ExtHostTransport.pump(extClient);
       closed^;
     });
     expect.bool(closed^).toBe(false);
   });
   test("basic extension activation", _ => {
     let setup = Setup.init();
     let rootPath = Rench.Environment.getWorkingDirectory();
     let testExtensionsPath =
       Rench.Path.join(rootPath, "test/test_extensions");
     let extensions =
       ExtensionScanner.scan(testExtensionsPath)
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
     let extClient = ExtHostTransport.start(~initData, ~onMessage, setup);
     Oni_Core.Utility.waitForCondition(() => {
       ExtHostTransport.pump(extClient);
       gotWillActivateMessage^;
     });
     Oni_Core.Utility.waitForCondition(() => {
       ExtHostTransport.pump(extClient);
       gotDidActivateMessage^;
     });
     ExtHostTransport.close(extClient);
   });
 });

 */
