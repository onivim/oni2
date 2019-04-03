open Oni_Core;
open Oni_Core_Test;
open Oni_Extensions;

open TestFramework;

open ExtensionClientHelper;
open ExtensionHostProtocol;
open ExtensionHostProtocol.OutgoingNotifications;

module JsonInformationMessageFormat = {
    [@deriving (show({with_path: false}), yojson({strict: false, exn: true}))]
    type t = {
       [@key "type"]
       messageType: string,
       filename: string,
    }
};

describe("Extension Client", ({describe, _}) => {
  describe("commands", ({test, _}) =>
    test("executes simple command", _ =>
      withExtensionClient(api => {
        let waitForCommandRegistration =
          api.createWaiterForMessage(
            "MainThreadCommands", "$registerCommand", args =>
            switch (args) {
            | [`String("extension.helloWorld"), ..._] => true
            | _ => false
            }
          );
        let waitForShowMessage =
          api.createWaiterForMessage(
            "MainThreadMessageService", "$showMessage", _ =>
            true
          );

        api.start();

        waitForCommandRegistration();

        api.send(Commands.executeContributedCommand("extension.helloWorld"));

        waitForShowMessage();
      })
    )
  );

  describe("DocumentsAndEditors", ({test, _}) =>
    test("document added successfully", _ => /*({expect}) =>*/
      withExtensionClient(api => {
        let waitForCommandRegistration =
          api.createWaiterForMessage(
            "MainThreadCommands", "$registerCommand", args =>
            switch (args) {
            | [`String("extension.helloWorld"), ..._] => true
            | _ => false
            }
          );

        let waitForShowMessage =
          api.createWaiterForMessage(
            "MainThreadMessageService", "$showMessage", args =>
            switch (args) {
            | [_, `String(s), ..._] =>
              let json = Yojson.Safe.from_string(s);
            open JsonInformationMessageFormat;
              let info = JsonInformationMessageFormat.of_yojson_exn(json);

              String.equal(info.filename, "test.txt") && String.equal(info.messageType, "workspace.onDidOpenTextDocument");
            | _ => failwith("Unknown message");
            }
          );

        api.start();
        waitForCommandRegistration();
        api.send(
          DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
            ~removedDocuments=[],
            ~addedDocuments=[
              ModelAddedDelta.create(
                ~uri=Uri.createFromFilePath("test.txt"),
                ~lines=["hello world"],
                ~modeId="test_language",
                ~isDirty=false,
                (),
              ),
            ],
            (),
          ),
        );

        waitForShowMessage();
      })
    )
  );

  describe("lifecycle", ({test, _}) => {
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
        ExtensionHostClient.close(extClient);
      })
    );
    test("doesn't die after a few seconds", ({expect}) => {
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
      let extClient =
        ExtensionHostClient.start(~onInitialized, ~onClosed, setup);
      Oni_Core.Utility.waitForCondition(() => {
        ExtensionHostClient.pump(extClient);
        initialized^;
      });
      expect.bool(initialized^).toBe(true);
      ExtensionHostClient.close(extClient);
      Oni_Core.Utility.waitForCondition(() => {
        ExtensionHostClient.pump(extClient);
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
             ExtensionHostInitData.ExtensionInfo.ofScannedExtension(ext)
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
      let initData = ExtensionHostInitData.create(~extensions, ());
      let extClient = ExtensionHostClient.start(~initData, ~onMessage, setup);
      Oni_Core.Utility.waitForCondition(() => {
        ExtensionHostClient.pump(extClient);
        gotWillActivateMessage^;
      });
      Oni_Core.Utility.waitForCondition(() => {
        ExtensionHostClient.pump(extClient);
        gotDidActivateMessage^;
      });
      ExtensionHostClient.close(extClient);
    });
  });
});
