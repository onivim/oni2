open EditorCoreTypes;
open Oni_Core;
open Oni_Extensions;

open TestFramework;

open ExtensionClientHelper;
open ExtHostProtocol;

let normalizeExpectedPath = path =>
  // Windows does a normalization of slashes in the ext host... we need to match this logic
  // for our verification. For example, if we send it a path like /root/test.txt, it'll send us
  // back \\root\\test.txt
  if (Sys.win32) {
    path |> String.split_on_char('/') |> String.concat("\\");
  } else {
    path;
  };

describe("ExtHostClient", ({describe, _}) => {
  describe("activation", ({test, _}) => {
    test("activates by language", ({expect}) => {
      let activations: ref(list(string)) = ref([]);
      let onDidActivateExtension = id => activations := [id, ...activations^];

      let isExpectedExtensionActivated = () =>
        isStringValueInList(activations, "oni-activation-events-test");

      withExtensionClient(
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
      withExtensionClient(
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

      withExtensionClient(
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
    test("document added successfully", ({expect}) => {
      let registeredCommands = empty();
      let messages = emptyInfoMsgs();

      let onShowMessage = appendInfoMsg(messages);
      let onRegisterCommand = append(registeredCommands);

      let isExpectedCommandRegistered = () =>
        isStringValueInList(registeredCommands, "extension.helloWorld");

      let didGetOpenMessage = () =>
        doesInfoMessageMatch(messages, info =>
          String.equal(
            info.filename,
            "/root/test.txt" |> normalizeExpectedPath,
          )
          && String.equal(info.messageType, "workspace.onDidOpenTextDocument")
        );

      withExtensionClient(
        ~onRegisterCommand,
        ~onShowMessage,
        client => {
          Waiters.wait(isExpectedCommandRegistered, client);
          expect.bool(isExpectedCommandRegistered()).toBe(true);

          ExtHostClient.addDocument(
            createInitialDocumentModel(
              ~lines=["Hello world"],
              ~path="/root/test.txt",
              (),
            ),
            client,
          );
          Waiters.wait(didGetOpenMessage, client);
          expect.bool(didGetOpenMessage()).toBe(true);
        },
      );
    });

    test("document updated successfully", ({expect}) => {
      let registeredCommands = empty();
      let messages = emptyInfoMsgs();

      let onShowMessage = appendInfoMsg(messages);
      let onRegisterCommand = append(registeredCommands);

      let isExpectedCommandRegistered = () =>
        isStringValueInList(registeredCommands, "extension.helloWorld");

      let didGetOpenMessage = () =>
        doesInfoMessageMatch(messages, info =>
          String.equal(
            info.filename,
            "/root/test.txt" |> normalizeExpectedPath,
          )
          && String.equal(info.messageType, "workspace.onDidOpenTextDocument")
        );

      let didGetUpdateMessage = () =>
        doesInfoMessageMatch(messages, info =>
          String.equal(
            info.filename,
            "/root/test.txt" |> normalizeExpectedPath,
          )
          && String.equal(
               info.messageType,
               "workspace.onDidChangeTextDocument",
             )
          && String.equal(
               info.fullText,
               "Greetings" ++ Eol.toString(Eol.default) ++ "world",
             )
        );

      withExtensionClient(
        ~onShowMessage,
        ~onRegisterCommand,
        client => {
          Waiters.wait(isExpectedCommandRegistered, client);
          expect.bool(isExpectedCommandRegistered()).toBe(true);

          ExtHostClient.addDocument(
            createInitialDocumentModel(
              ~lines=["hello", "world"],
              ~path="/root/test.txt",
              (),
            ),
            client,
          );
          Waiters.wait(didGetOpenMessage, client);
          expect.bool(didGetOpenMessage()).toBe(true);

          let contentChange =
            ModelContentChange.create(
              ~range=
                Range.create(
                  ~start=
                    Location.create(~line=Index.zero, ~column=Index.zero),
                  ~stop=
                    Location.create(
                      ~line=Index.zero,
                      ~column=Index.(zero + 5),
                    ),
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
          ExtHostClient.updateDocument(
            Uri.fromPath("/root/test.txt"),
            modelChangedEvent,
            ~dirty=true,
            client,
          );

          Waiters.wait(didGetUpdateMessage, client);
          expect.bool(didGetUpdateMessage()).toBe(true);
        },
      );
    });
  });
});
