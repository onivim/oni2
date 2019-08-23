open Revery;
open Oni_Core;
open Oni_Core_Test;
open Oni_Extensions;
open TestFramework;
open Oni_Extensions.TextmateClient;
open Oni_Extensions.ExtensionContributions.Grammar;

let reasonSyntaxPath = (setup: Setup.t) =>
  setup.bundledExtensionsPath ++ "/vscode-reasonml/syntaxes/reason.json";
let testThemePath = (setup: Setup.t) =>
  setup.bundledExtensionsPath ++ "/oni-test/theme1.json";
/* let testThemePath = (setup: Setup.t) => */
/* setup.bundledExtensionsPath ++ "/onedark-pro/themes/OneDark-Pro.json"; */

describe("Textmate Service", ({test, _}) => {
  test("receive init message", ({expect}) =>
    Helpers.repeat(() => {
      let setup = Setup.init();

      let gotInitNotification = ref(false);
      let gotClosedNotification = ref(false);

      let onInitialized = () => gotInitNotification := true;
      let onClosed = () => gotClosedNotification := true;
      let onTokens = _ => ();

      let tmClient =
        TextmateClient.start(~onClosed, ~onInitialized, ~onTokens, setup, []);

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

  exception TextmateServiceCloseException(string);

  let withTextmateClient =
      (~onColorMap, ~onScopeLoaded, ~onTokens, initData, f) => {
    let setup = Setup.init();
    let tmClient =
      TextmateClient.start(
        ~onColorMap,
        ~onScopeLoaded,
        ~onTokens,
        setup,
        initData,
      );

    f(tmClient);

    let result = TextmateClient.close(tmClient);
    switch (result) {
    | (_, Unix.WEXITED(_)) => ()
    | _ =>
      raise(TextmateServiceCloseException("Error closing textmate service"))
    };
  };

  test("load grammar / scope", ({expect}) => {
    let setup = Setup.init();

    let onColorMap = _ => ();
    let onTokens = _ => ();
    let gotScopeLoadedMessage = ref(false);

    let onScopeLoaded = (s: string) =>
      switch (s) {
      | "source.reason" => gotScopeLoadedMessage := true
      | _ => prerr_endline("Unknown scope: " ++ s)
      };

    withTextmateClient(
      ~onColorMap,
      ~onScopeLoaded,
      ~onTokens,
      [
        {
          scopeName: "source.reason",
          path: reasonSyntaxPath(setup),
          treeSitterPath: None,
          language: None,
        },
      ],
      tmClient => {
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

        switch (tokenizeResult) {
        | Some(v) =>
          expect.int(List.length(v.tokens)).toBe(5);

          /* Only a single color since no theme set */
          expect.int(List.length(v.colors)).toBe(1);

          let onlyColor = List.hd(v.colors);
          expect.int(onlyColor.index).toBe(0);
          expect.int(onlyColor.foregroundColor).toBe(0);
          expect.int(onlyColor.backgroundColor).toBe(1);

          let firstResult = List.hd(v.tokens);
          expect.int(firstResult.startIndex).toBe(0);
          expect.int(firstResult.endIndex).toBe(3);
        | None => expect.string("fail").toEqual("no token result")
        };
      },
    );
  });

  test("load theme and get colormap", ({expect}) => {
    let setup = Setup.init();
    let colorMap: ref(option(ColorMap.t)) = ref(None);

    let onColorMap = c => colorMap := Some(c);
    let onScopeLoaded = _ => ();
    let onTokens = _ => ();

    withTextmateClient(
      ~onColorMap,
      ~onScopeLoaded,
      ~onTokens,
      [
        {
          scopeName: "source.reason",
          path: reasonSyntaxPath(setup),
          treeSitterPath: None,
          language: None,
        },
      ],
      tmClient => {
        TextmateClient.setTheme(tmClient, testThemePath(setup));

        Oni_Core.Utility.waitForCondition(() => {
          TextmateClient.pump(tmClient);
          switch (colorMap^) {
          | Some(_) => true
          | None => false
          };
        });

        switch (colorMap^) {
        | Some(c) =>
          let firstColor = ColorMap.get(c, 0, Colors.black, Colors.white);
          expect.float(firstColor.r).toBeCloseTo(0.0);
          expect.float(firstColor.g).toBeCloseTo(0.0);
          expect.float(firstColor.b).toBeCloseTo(0.0);

          let secondColor = ColorMap.get(c, 1, Colors.black, Colors.white);
          expect.float(secondColor.r).toBeCloseTo(1.0);
          expect.float(secondColor.g).toBeCloseTo(1.0);
          expect.float(secondColor.b).toBeCloseTo(1.0);
        | None => expect.string("Failed").toEqual("get color map")
        };
      },
    );
  });

  test("theme applied to tokenization", ({expect}) => {
    let setup = Setup.init();
    let colorMap: ref(option(ColorMap.t)) = ref(None);

    let onColorMap = c => colorMap := Some(c);
    let onScopeLoaded = _ => ();
    let onTokens = _ => ();

    withTextmateClient(
      ~onColorMap,
      ~onScopeLoaded,
      ~onTokens,
      [
        {
          scopeName: "source.reason",
          path: reasonSyntaxPath(setup),
          treeSitterPath: None,
          language: None,
        },
      ],
      tmClient => {
        TextmateClient.setTheme(tmClient, testThemePath(setup));

        Oni_Core.Utility.waitForCondition(() => {
          TextmateClient.pump(tmClient);
          switch (colorMap^) {
          | Some(_) => true
          | None => false
          };
        });

        let tokenizeResult =
          TextmateClient.tokenizeLineSync(
            tmClient,
            "source.reason",
            "let abc = 100;",
          );

        switch (tokenizeResult, colorMap^) {
        | (Some(v), Some(cm)) =>
          expect.int(List.length(v.colors)).toBe(6);
          let firstChild = List.hd(v.colors);
          let firstColor =
            ColorMap.get(
              cm,
              firstChild.foregroundColor,
              Colors.black,
              Colors.white,
            );
          expect.float(firstColor.r).toBeCloseTo(1.0);
          expect.float(firstColor.g).toBeCloseTo(0.0);
          expect.float(firstColor.b).toBeCloseTo(0.0);

        | _ => expect.string("fail").toEqual("theme or colormap didn't load")
        };
      },
    );
  });
});
