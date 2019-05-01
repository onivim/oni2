open Oni_Core;
open Oni_Core_Test;
open Oni_Extensions;

open TestFramework;

open ExtensionClientHelper;
open ExtHostProtocol;
open ExtHostProtocol.OutgoingNotifications;

module JsonInformationMessageFormat = {
  [@deriving (show({with_path: false}), yojson({strict: false, exn: true}))]
  type t = {
    [@key "type"]
    messageType: string,
    filename: string,
    fullText: string,
  };
};

describe("ExtHostClient", ({describe, _}) => {
  describe("activation", ({test, _}) => {
    test("activates by language", _ =>
      withExtensionClient(api => {
        let waitForActivationMessage =
          api
          |> Waiters.createMessageWaiter(s => String.equal(s, "Activated!"));

        api.start();

        api.send(ExtensionService.activateByEvent("onLanguage:testlang"));

        waitForActivationMessage();
      })
    );

    test("activates by command", _ =>
      withExtensionClient(api => {
        let waitForActivationMessage =
          api
          |> Waiters.createMessageWaiter(s => String.equal(s, "Activated!"));

        api.start();

        api.send(
          ExtensionService.activateByEvent(
            "onCommand:extension.activationTest",
          ),
        );

        waitForActivationMessage();
      })
    );
  });

  describe("commands", ({test, _}) =>
    test("executes simple command", _ =>
      withExtensionClient(api => {
        let waitForCommandRegistration =
          api
          |> Waiters.createCommandRegistrationWaiter("extension.helloWorld");
        let waitForShowMessage = api |> Waiters.createMessageWaiter(_ => true);

        api.start();

        waitForCommandRegistration();

        api.send(Commands.executeContributedCommand("extension.helloWorld"));

        waitForShowMessage();
      })
    )
  );

  describe("DocumentsAndEditors", ({test, _}) => {
    let createInitialDocumentModel = (~lines, ~path, ()) => {
      ModelAddedDelta.create(
        ~uri=Uri.fromPath(path),
        ~lines,
        ~modeId="test_language",
        ~isDirty=false,
        (),
      );
    };
    test("document added successfully", _
      /*({expect}) =>*/
      =>
        withExtensionClient(api => {
          let waitForCommandRegistration =
            api
            |> Waiters.createCommandRegistrationWaiter("extension.helloWorld");

          let waitForOpenMessage =
            api
            |> Waiters.createMessageWaiter(s => {
                 let json = Yojson.Safe.from_string(s);
                 open JsonInformationMessageFormat;
                 let info = JsonInformationMessageFormat.of_yojson_exn(json);

                 String.equal(info.filename, "test.txt")
                 && String.equal(
                      info.messageType,
                      "workspace.onDidOpenTextDocument",
                    );
               });

          api.start();
          waitForCommandRegistration();
          api.send(
            DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
              ~removedDocuments=[],
              ~addedDocuments=[
                createInitialDocumentModel(
                  ~lines=["Hello world"],
                  ~path="test.txt",
                  (),
                ),
              ],
              (),
            ),
          );

          waitForOpenMessage();
        })
      );

    test("document updated successfully", _ =>
      withExtensionClient(api => {
        let waitForCommandRegistration =
          api
          |> Waiters.createCommandRegistrationWaiter("extension.helloWorld");

        let waitForOpenMessage =
          api
          |> Waiters.createMessageWaiter(s => {
               let json = Yojson.Safe.from_string(s);
               open JsonInformationMessageFormat;
               let info = JsonInformationMessageFormat.of_yojson_exn(json);

               String.equal(info.filename, "test.txt")
               && String.equal(
                    info.messageType,
                    "workspace.onDidOpenTextDocument",
                  );
             });

        let waitForUpdateMessage =
          api
          |> Waiters.createMessageWaiter(s => {
               let json = Yojson.Safe.from_string(s);
               open JsonInformationMessageFormat;
               let info = JsonInformationMessageFormat.of_yojson_exn(json);

               String.equal(info.filename, "test.txt")
               && String.equal(
                    info.messageType,
                    "workspace.onDidChangeTextDocument",
                  )
               && String.equal(
                    info.fullText,
                    "Greetings" ++ Eol.toString(Eol.default) ++ "world",
                  );
             });

        api.start();
        waitForCommandRegistration();
        api.send(
          DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
            ~removedDocuments=[],
            ~addedDocuments=[
              createInitialDocumentModel(
                ~lines=["hello", "world"],
                ~path="test.txt",
                (),
              ),
            ],
            (),
          ),
        );

        waitForOpenMessage();

        let contentChange =
          ModelContentChange.create(
            ~range=
              Range.create(
                ~startLine=ZeroBasedIndex(0),
                ~endLine=ZeroBasedIndex(0),
                ~startCharacter=ZeroBasedIndex(0),
                ~endCharacter=ZeroBasedIndex(5),
                (),
              ),
            ~text="Greetings",
            (),
          );

        let modelChangedEvent =
          ModelChangedEvent.create(
            ~changes=[contentChange],
            ~eol=Eol.default,
            ~versionId=1,
            (),
          );

        api.send(
          Documents.acceptModelChanged(
            Uri.fromPath("test.txt"),
            modelChangedEvent,
            true,
          ),
        );

        waitForUpdateMessage();
      })
    );
  });
});
