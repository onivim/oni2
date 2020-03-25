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

let getKeyFromSDL: string => EditorInput.KeyPress.t =
  key => {
    let scancode = Sdl2.Scancode.ofName(key);
    let keycode = Sdl2.Keycode.ofName(key);
    EditorInput.{keycode, scancode, modifiers: EditorInput.Modifiers.none};
  };

let contextWithEditorTextFocus =
  Seq.return(("editorTextFocus", true)) |> Hashtbl.of_seq;

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
      expect.int(errorCount(result)).toBe(0);

      result
      |> Utility.ResultEx.tapError(err => failwith(err))
      |> Result.iter(((bindings, _)) => {
           let (_bindings, effects) =
             keyDown(
               ~context=contextWithEditorTextFocus,
               ~key=getKeyFromSDL("F2"),
               bindings,
             );

           expect.equal(effects, [Execute("explorer.toggle")]);
         });
    });
    test("regression test: #1160 (legacy binding)", ({expect, _}) => {
      let result = of_yojson_with_errors(regressionTest1160);
      expect.bool(isOk(result)).toBe(true);
      expect.int(bindingCount(result)).toBe(4);
      expect.int(errorCount(result)).toBe(0);

      let validateKeyResultsInCommand = ((key, modifiers, cmd)) => {
        result
        |> Result.iter(((bindings, _)) => {
             let key = {...getKeyFromSDL(key), modifiers};
             let (_bindings, effects) =
               keyDown(~context=contextWithEditorTextFocus, ~key, bindings);
             expect.equal(effects, [Execute(cmd)]);
           });
      };

      let modifier = (~control, ~shift, ~meta) => {
        ...EditorInput.Modifiers.none,
        control,
        shift,
        meta,
      };

      let cases = [
        (
          "p",
          modifier(~control=true, ~shift=false, ~meta=false),
          "workbench.action.quickOpen",
        ),
        (
          "p",
          modifier(~control=false, ~shift=false, ~meta=true),
          "workbench.action.quickOpen",
        ),
        (
          "p",
          modifier(~control=true, ~shift=true, ~meta=false),
          "workbench.action.showCommands",
        ),
        (
          "p",
          modifier(~control=false, ~shift=true, ~meta=true),
          "workbench.action.showCommands",
        ),
      ];

      cases |> List.iter(validateKeyResultsInCommand);
    });
  })
});
