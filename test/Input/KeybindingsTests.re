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

let regressionTest1152 =
  {|
{
bindings: [{key: "<F2>", command: "explorer.toggle", when: [["editorTextFocus"]]}]
}
|}
  |> Yojson.Safe.from_string;

let regressionTest1160 =
  {|
{
bindings: [
  {key: "<C-P>", command: "quickOpen.open", when: [["editorTextFocus"]]},
  {key: "<D-P>", command: "quickOpen.open", when: [["editorTextFocus"]]},
  {key: "<S-C-P>", command: "commandPalette.open", when: [["editorTextFocus"]]},
  {key: "<D-S-P>", command: "commandPalette.open", when: [["editorTextFocus"]]}
]
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
  | Ok((bindings, _)) => count(bindings)
  | Error(_) => 0
  };

let getFirstBinding = v =>
  switch (v) {
  | Ok(([firstBinding], _)) => firstBinding
  | _ => failwith("No binding found")
  };

let getNthBinding = (~index, v) => {
  switch (v) {
  | Ok((bindings, _)) => List.nth(bindings, index)
  | _ => failwith("No binding found")
  };
};

let errorCount = v =>
  switch (v) {
  | Ok((_, errors)) => List.length(errors)
  | Error(_) => 0
  };

describe("Keybindings", ({describe, _}) => {
  describe("parsing", ({test, _}) => {
    test("basic key binding", ({expect, _}) => {
      let result = of_yojson_with_errors(keybindingsJSON);
      expect.bool(isOk(result)).toBe(true);
      expect.int(bindingCount(result)).toBe(1);
      expect.int(errorCount(result)).toBe(0);
    });
    test("basic key binding with comment", ({expect, _}) => {
      let result = of_yojson_with_errors(keybindingsJSONWithComment);
      expect.bool(isOk(result)).toBe(true);
      expect.int(bindingCount(result)).toBe(1);
      expect.int(errorCount(result)).toBe(0);
    });
    test("legacy keybinding with new expression", ({expect, _}) => {
      let result =
        of_yojson_with_errors(legacyKeyBindingsWithNewExpressionJSON);
      expect.bool(isOk(result)).toBe(true);
      expect.int(bindingCount(result)).toBe(1);
      expect.int(errorCount(result)).toBe(0);
    });
    test("regression test: #1152 (legacy expression)", ({expect, _}) => {
      let result = of_yojson_with_errors(regressionTest1152);
      expect.bool(isOk(result)).toBe(true);
      expect.int(bindingCount(result)).toBe(1);
      expect.int(errorCount(result)).toBe(0);

      let binding = getFirstBinding(result);
      expect.equal(
        binding,
        Keybinding.{
          key: "<F2>",
          command: "explorer.toggle",
          condition: WhenExpr.Or([And([Defined("editorTextFocus")])]),
        },
      );
    });
    test("regression test: #1160 (legacy binding)", ({expect, _}) => {
      let result = of_yojson_with_errors(regressionTest1160);
      expect.bool(isOk(result)).toBe(true);
      expect.int(bindingCount(result)).toBe(4);
      expect.int(errorCount(result)).toBe(0);

      Keybinding.(
        {
          let binding0 = getNthBinding(~index=0, result);
          let binding1 = getNthBinding(~index=1, result);
          let binding2 = getNthBinding(~index=2, result);
          let binding3 = getNthBinding(~index=3, result);

          // Validate quickOpen.open gets upgraded to workbench.action.quickOpen
          expect.equal(binding0.command, "workbench.action.quickOpen");
          expect.equal(binding1.command, "workbench.action.quickOpen");

          // Validate commandPalette.open gets upgraded to workbench.action.quickOpen
          expect.equal(binding2.command, "workbench.action.showCommands");
          expect.equal(binding3.command, "workbench.action.showCommands");
        }
      );
    });
  })
});
