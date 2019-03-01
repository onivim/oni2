open Oni_Core;
open TestFramework;

describe("Textmate Service", ({test, _}) => {
  test("receive init message", ({expect}) =>
    Helpers.repeat(() => {
      let setup = Setup.init();

      let gotInitNotification = ref(false);
      let gotClosedNotification = ref(false);

      let onInitialized = () => gotInitNotification := true;
      let onClosed = () => gotClosedNotification := true;

      let tmClient =
        TextmateClient.start(~onClosed, ~onInitialized, setup, []);

      Oni_Core.Utility.waitForCondition(() => {
        TextmateClient.pump(tmClient);
        gotInitNotification^;
      });
      expect.bool(gotInitNotification^).toBe(true);

      let result = TextmateClient.close(tmClient);

      Oni_Core.Utility.waitForCondition(() => {
        TextmateClient.pump(tmClient);
        gotClosedNotification^;
      });
      expect.bool(gotClosedNotification^).toBe(true);

      switch (result) {
      | (_, Unix.WEXITED(v)) => expect.int(v).toBe(0)
      | _ =>
        expect.string("Expected WEXITED").toEqual("Got different exit state")
      };
    })
  );

  test("load grammar / scope", ({expect}) => {
    let setup = Setup.init();

    let gotScopeLoadedMessage = ref(false);

    let onScopeLoaded = (s: string) =>
      switch (s) {
      | "source.reason" => gotScopeLoadedMessage := true
      | _ => prerr_endline("Unknown scope: " ++ s)
      };

    let tmClient =
      TextmateClient.start(
        ~onScopeLoaded,
        setup,
        [
          {
            scopeName: "source.reason",
            path:
              setup.bundledExtensionsPath
              ++ "/vscode-reasonml/syntaxes/reason.json",
          },
        ],
      );

    TextmateClient.preloadScope(tmClient, "source.reason");

    Oni_Core.Utility.waitForCondition(() => {
      TextmateClient.pump(tmClient);
      gotScopeLoadedMessage^;
    });
    expect.bool(gotScopeLoadedMessage^).toBe(true);

    let tokenizeResult =
      TextmateClient.tokenizeLineSync(
        tmClient,
        "source.reason",
        "let abc = 100;",
      );

    expect.int(List.length(tokenizeResult)).toBe(5);

    let firstResult = List.hd(tokenizeResult);
    expect.int(firstResult.startIndex).toBe(0);
    expect.int(firstResult.endIndex).toBe(3);

    let result = TextmateClient.close(tmClient);
    switch (result) {
    | (_, Unix.WEXITED(v)) => expect.int(v).toBe(0)
    | _ =>
      expect.string("Expected WEXITED").toEqual("Got different exit state")
    };
  });
});
