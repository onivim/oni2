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

let empty: unit => ref(list(string)) = () => ref([]);

let clear = (r: ref(list(string))) => r := [];

let append = (r: ref(list(string)), s: string) => r := [s, ...r^];

let any = (r: ref(list(string)), ()) => List.length(r^) > 0;

let isStringValueInList = (r: ref(list(string)), match: string) => {
  let l = r^ |> List.filter(id => String.equal(id, match)) |> List.length;
  l > 0;
};

describe("ExtHostClient", ({describe, _}) => {
  describe("activation", ({test, _}) => {
    test("activates by language", ({expect}) => {
      let activations: ref(list(string)) = ref([]);
      let onDidActivateExtension = id => activations := [id, ...activations^];

      let isExpectedExtensionActivated = () =>
        isStringValueInList(activations, "oni-activation-events-test");

      withExtensionClient2(
        ~onDidActivateExtension,
        client => {
          ExtHostClient.activateByEvent("onLanguage:testlang", client);

          Waiters.wait(isExpectedExtensionActivated, client);
          expect.bool(isExpectedExtensionActivated()).toBe(true);
        },
      );
    });

    test("activates by command", ({expect}) => {
      let activations: ref(list(string)) = ref([]);
      let onDidActivateExtension = id => activations := [id, ...activations^];

      let isExpectedExtensionActivated = () =>
        isStringValueInList(activations, "oni-activation-events-test");
      withExtensionClient2(
        ~onDidActivateExtension,
        client => {
          ExtHostClient.activateByEvent(
            "onCommand:extension.activationTest",
            client,
          );

          Waiters.wait(isExpectedExtensionActivated, client);
          expect.bool(isExpectedExtensionActivated()).toBe(true);
        },
      );
    });
  });

  describe("commands", ({test, _}) =>
    test("executes simple command", ({expect}) => {
      let registeredCommands = empty();
      let messages = empty();

      let onShowMessage = append(messages);
      let onRegisterCommand = append(registeredCommands);

      let isExpectedCommandRegistered = () =>
        isStringValueInList(registeredCommands, "extension.helloWorld");

      let anyMessages = any(messages);

      withExtensionClient2(
        ~onShowMessage,
        ~onRegisterCommand,
        client => {
          Waiters.wait(isExpectedCommandRegistered, client);
          expect.bool(isExpectedCommandRegistered()).toBe(true);

          clear(messages);

          ExtHostClient.executeContributedCommand(
            "extension.helloWorld",
            client,
          );

          Waiters.wait(anyMessages, client);
          expect.bool(anyMessages()).toBe(true);
        },
      );
    })
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
