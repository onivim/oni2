open TestFramework;

open Oni_Input.Keybindings;

let keybindingsJSON =
  {|
[
  {key: "<C-V>", command: "quickOpen.show", when: "editorTextFocus"}
]
|}
  |> Yojson.Safe.from_string;

let keybindingsJSONWithComment =
  {|
[
  // show quickopen
  {key: "<C-V>", command: "quickOpen.show", when: "editorTextFocus"}
]
|}
  |> Yojson.Safe.from_string;

let legacyKeyBindingsWithNewExpressionJSON =
  {|
{
bindings: [{key: "<C-V>", command: "quickOpen.show", when: "editorTextFocus"}]
}
|}
  |> Yojson.Safe.from_string;

let isOk = v =>
  switch (v) {
  | Ok(_) => true
  | Error(_) => false
  };

let bindingCount = v =>
  switch (v) {
  | Ok((bindings, _)) => List.length(bindings)
  | Error(_) => 0
  };

let errorCount = v =>
  switch (v) {
  | Ok((_, errors)) => List.length(errors)
  | Error(_) => 0
  };

describe("Keybindings", ({describe, _}) => {
  describe("parsing", ({test, _}) => {
    test("basic key binding", ({expect}) => {
      let result = of_yojson_with_errors(keybindingsJSON);
      expect.bool(isOk(result)).toBe(true);
      expect.int(bindingCount(result)).toBe(1);
      expect.int(errorCount(result)).toBe(0);
    });
    test("basic key binding with comment", ({expect}) => {
      let result = of_yojson_with_errors(keybindingsJSONWithComment);
      expect.bool(isOk(result)).toBe(true);
      expect.int(bindingCount(result)).toBe(1);
      expect.int(errorCount(result)).toBe(0);
    });
    test("legacy keybinding with new expression", ({expect}) => {
      let result =
        of_yojson_with_errors(legacyKeyBindingsWithNewExpressionJSON);
      expect.bool(isOk(result)).toBe(true);
      expect.int(bindingCount(result)).toBe(1);
      expect.int(errorCount(result)).toBe(0);
    });
  })
});
