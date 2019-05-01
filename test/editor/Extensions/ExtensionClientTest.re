open Oni_Core;
open Oni_Core_Test;
open Oni_Extensions;

open TestFramework;

open ExtensionClientHelper;
open ExtHostProtocol;

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
let emptyInfoMsgs: unit => ref(list(JsonInformationMessageFormat.t)) = () => ref([]);

let clear = (r: ref(list(string))) => r := [];

let appendInfoMsg = (r, s) => {

													let json = Yojson.Safe.from_string(s);
													let info = JsonInformationMessageFormat.of_yojson(json);

		switch (info) {
		| Ok(v) => r := [v, ...r^];
		| _ => ()
		};
};

let doesInfoMessageMatch = (r: ref(list(JsonInformationMessageFormat.t)), f) => {
	let l = r^
	|> List.filter(f)
	|> List.length;
	l > 0;
};

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
    test("document added successfully", ({expect}) => {
      let registeredCommands = empty();
      let messages = emptyInfoMsgs();

      let onShowMessage = appendInfoMsg(messages);
      let onRegisterCommand = append(registeredCommands);

      let isExpectedCommandRegistered = () =>
        isStringValueInList(registeredCommands, "extension.helloWorld");

      let didGetOpenMessage = () => doesInfoMessageMatch(messages, (info) => {
                 String.equal(info.filename, "test.txt")
                 && String.equal(
                      info.messageType,
                      "workspace.onDidOpenTextDocument",
                    );
						});

        withExtensionClient2(~onRegisterCommand, ~onShowMessage, client => {
		  Waiters.wait(isExpectedCommandRegistered, client);
		  expect.bool(isExpectedCommandRegistered()).toBe(true);

		 ExtHostClient.addDocument(
                createInitialDocumentModel(
                  ~lines=["Hello world"],
                  ~path="test.txt",
                  (),
                ), client);
		  Waiters.wait(didGetOpenMessage, client);
		  expect.bool(didGetOpenMessage()).toBe(true);
        })
      });

    test("document updated successfully", ({expect}) => {
      let registeredCommands = empty();
      let messages = emptyInfoMsgs();

      let onShowMessage = appendInfoMsg(messages);
      let onRegisterCommand = append(registeredCommands);

      let isExpectedCommandRegistered = () =>
        isStringValueInList(registeredCommands, "extension.helloWorld");

      let didGetOpenMessage = () => doesInfoMessageMatch(messages, (info) => {
                 String.equal(info.filename, "test.txt")
                 && String.equal(
                      info.messageType,
                      "workspace.onDidOpenTextDocument",
                    );
						});

      let didGetUpdateMessage = () => doesInfoMessageMatch(messages, (info) => {
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

      withExtensionClient2(~onShowMessage, ~onRegisterCommand, client => {

		  Waiters.wait(isExpectedCommandRegistered, client);
		  expect.bool(isExpectedCommandRegistered()).toBe(true);

		ExtHostClient.addDocument(
              createInitialDocumentModel(
                ~lines=["hello", "world"],
                ~path="test.txt",
                (),
              ),
														client);
		  Waiters.wait(didGetOpenMessage, client);
		  expect.bool(didGetOpenMessage()).toBe(true);

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
		ExtHostClient.updateDocument(
            Uri.fromPath("test.txt"),
            modelChangedEvent,
            true,
			client);

		  Waiters.wait(didGetUpdateMessage, client);
		  expect.bool(didGetUpdateMessage()).toBe(true);
      })
    });
  });
});
