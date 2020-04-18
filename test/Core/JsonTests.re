open EditorCoreTypes;
open Oni_Core;
open TestFramework;

module Json = Oni_Core.Json;

describe("Json", ({describe, _}) =>
  describe("Error", ({describe, _}) =>
    describe("ofString", ({test, _}) => {
      test("invalid message", ({expect, _}) => {
        let maybeError = Json.Error.ofString("some error that doesn't match");

        expect.equal(maybeError, None);
      });
      test("valid error", ({expect, _}) => {
        let validError = {|
Line 6, bytes 2-35:
Expected ',' or '}' but found something else
      |};

        let maybeError = Json.Error.ofString(validError);
        let range =
          Range.{
            start: {
              line: Index.(zero + 5),
              column: Index.(zero + 2),
            },
            stop: {
              line: Index.(zero + 5),
              column: Index.(zero + 35),
            },
          };
        let message = "Expected ',' or '}' but found something else";
        expect.equal(maybeError, Some({range, message}));
      });
    })
  )
);
