open TestFramework;
open EditorInput;

let defaultParse = Matcher.parse(~explicitShiftKeyNeeded=true);

let defaultParseImplicitShiftKey =
  Matcher.parse(~explicitShiftKeyNeeded=false);

let modifiersControl = {...Modifiers.none, control: true};

let modifiersShift = {...Modifiers.none, shift: true};

let keyPress = (~modifiers=Modifiers.none, key) =>
  KeyPress.physicalKey(~key, ~modifiers);

let specialKey = KeyPress.specialKey;

describe("Matcher", ({describe, _}) => {
  describe("parser", ({test, _}) => {
    test("all keys", ({expect, _}) => {
      open Matcher;
      // Exercise full set of keys described here:
      // https://code.visualstudio.com/docs/getstarted/keybindings#_accepted-keys
      let cases = [
        // TODO: Add production cases here
        ("a", keyPress(Key.Character(Uchar.of_char('a')))),
        ("A", keyPress(Key.Character(Uchar.of_char('a')))), // Because implicit shift is false
        ("0", keyPress(Key.Character(Uchar.of_char('0')))),
        ("9", keyPress(Key.Character(Uchar.of_char('9')))),
        ("`", keyPress(Key.Character(Uchar.of_char('`')))),
        ("-", keyPress(Key.Character(Uchar.of_char('-')))),
        ("=", keyPress(Key.Character(Uchar.of_char('=')))),
        ("[", keyPress(Key.Character(Uchar.of_char('[')))),
        ("]", keyPress(Key.Character(Uchar.of_char(']')))),
        ("\\", keyPress(Key.Character(Uchar.of_char('\\')))),
        (";", keyPress(Key.Character(Uchar.of_char(';')))),
        ("'", keyPress(Key.Character(Uchar.of_char('\'')))),
        (",", keyPress(Key.Character(Uchar.of_char(',')))),
        (".", keyPress(Key.Character(Uchar.of_char('.')))),
        ("/", keyPress(Key.Character(Uchar.of_char('/')))),
        ("ö", keyPress(Key.Character(Uchar.of_int(246)))),
        ("tab", keyPress(Key.Tab)),
        ("ESC", keyPress(Key.Escape)),
        ("up", keyPress(Key.Up)),
        ("left", keyPress(Key.Left)),
        ("right", keyPress(Key.Right)),
        ("down", keyPress(Key.Down)),
        ("PageUp", keyPress(Key.PageUp)),
        ("pagedown", keyPress(Key.PageDown)),
        ("end", keyPress(Key.End)),
        ("home", keyPress(Key.Home)),
        ("enter", keyPress(Key.Return)),
        ("cr", keyPress(Key.Return)),
        ("escape", keyPress(Key.Escape)),
        ("EsCaPe", keyPress(Key.Escape)),
        ("space", keyPress(Key.Space)),
        ("bs", keyPress(Key.Backspace)),
        ("Bs", keyPress(Key.Backspace)),
        ("backspace", keyPress(Key.Backspace)),
        ("del", keyPress(Key.Delete)),
        ("delete", keyPress(Key.Delete)),
        ("pause", keyPress(Key.Pause)),
        ("capslock", keyPress(Key.CapsLock)),
        ("insert", keyPress(Key.Insert)),
        ("f0", keyPress(Key.Function(0))),
        ("F0", keyPress(Key.Function(0))),
        ("f19", keyPress(Key.Function(19))),
        ("F19", keyPress(Key.Function(19))),
        ("numpad0", keyPress(Key.NumpadDigit(0))),
        ("numpad9", keyPress(Key.NumpadDigit(9))),
        ("numpad_multiply", keyPress(Key.NumpadMultiply)),
        ("numpad_add", keyPress(Key.NumpadAdd)),
        ("numpad_separator", keyPress(Key.NumpadSeparator)),
        ("numpad_subtract", keyPress(Key.NumpadSubtract)),
        ("numpad_decimal", keyPress(Key.NumpadDecimal)),
        ("numpad_divide", keyPress(Key.NumpadDivide)),
        ("<plug>", specialKey(SpecialKey.Plug)),
        ("<Plug>", specialKey(SpecialKey.Plug)),
        ("<leader>", specialKey(SpecialKey.Leader)),
        ("<Leader>", specialKey(SpecialKey.Leader)),
        // Non-direct production keys - start of cases for #2114 / #2883
        (":", keyPress(Key.Character(Uchar.of_char(':')))),
        ("~", keyPress(Key.Character(Uchar.of_char('~')))),
        ("_", keyPress(Key.Character(Uchar.of_char('_')))),
        ("+", keyPress(Key.Character(Uchar.of_char('+')))),
        ("<gt>", keyPress(Key.Character(Uchar.of_char('>')))),
        ("<lt>", keyPress(Key.Character(Uchar.of_char('<')))),
      ];

      let runCase = case => {
        let (keyString, matcher) = case;
        let result = defaultParse(keyString);

        expect.equal(result, Ok(Sequence([matcher])));
      };

      let _: unit = cases |> List.iter(runCase);
      ();
    });
    test("simple parsing", ({expect, _}) => {
      let result = defaultParse("a");
      expect.equal(
        result,
        Ok(Sequence([keyPress(Key.Character(Uchar.of_char('a')))])),
      );

      let result = defaultParse("b");
      expect.equal(
        result,
        Ok(Sequence([keyPress(Key.Character(Uchar.of_char('b')))])),
      );

      let result = defaultParse("esc");
      expect.equal(result, Ok(Sequence([keyPress(Key.Escape)])));
    });
    test("all keys released", ({expect, _}) => {
      let result = defaultParse("<RELEASE>");
      expect.equal(result, Ok(AllKeysReleased));
    });
    // https://github.com/onivim/oni2/issues/3256
    test("gt #3256)", ({expect, _}) => {
      expect.equal(
        defaultParse("gt"),
        Ok(
          Sequence([
            keyPress(Key.Character(Uchar.of_char('g'))),
            keyPress(Key.Character(Uchar.of_char('t'))),
          ]),
        ),
      )
    });
    test("vim bindings", ({expect, _}) => {
      let result = defaultParse("<a>");
      expect.equal(
        result,
        Ok(Sequence([keyPress(Key.Character(Uchar.of_char('a')))])),
      );

      let result = defaultParse("<c-a>");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(
              ~modifiers=modifiersControl,
              Key.Character(Uchar.of_char('a')),
            ),
          ]),
        ),
      );

      let result = defaultParse("<S-a>");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(
              ~modifiers=modifiersShift,
              Key.Character(Uchar.of_char('a')),
            ),
          ]),
        ),
      );

      let result = defaultParse("<S-F12>");
      expect.equal(
        result,
        Ok(
          Sequence([keyPress(~modifiers=modifiersShift, Key.Function(12))]),
        ),
      );

      let result = defaultParse("<S-TAB>");
      expect.equal(
        result,
        Ok(Sequence([keyPress(~modifiers=modifiersShift, Key.Tab)])),
      );
    });
    test("vscode bindings", ({expect, _}) => {
      let result = defaultParse("Ctrl+a");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(
              ~modifiers=modifiersControl,
              Key.Character(Uchar.of_char('a')),
            ),
          ]),
        ),
      );

      let result = defaultParse("ctrl+a");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(
              ~modifiers=modifiersControl,
              Key.Character(Uchar.of_char('a')),
            ),
          ]),
        ),
      );
    });
    test("binding list", ({expect, _}) => {
      let result = defaultParse("ab");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(Key.Character(Uchar.of_char('a'))),
            keyPress(Key.Character(Uchar.of_char('b'))),
          ]),
        ),
      );

      let result = defaultParse("a b");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(Key.Character(Uchar.of_char('a'))),
            keyPress(Key.Character(Uchar.of_char('b'))),
          ]),
        ),
      );

      let result = defaultParse("<a>b");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(Key.Character(Uchar.of_char('a'))),
            keyPress(Key.Character(Uchar.of_char('b'))),
          ]),
        ),
      );
      let result = defaultParse("<a><b>");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(Key.Character(Uchar.of_char('a'))),
            keyPress(Key.Character(Uchar.of_char('b'))),
          ]),
        ),
      );

      let result = defaultParse("<c-a> Ctrl+b");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(
              ~modifiers=modifiersControl,
              Key.Character(Uchar.of_char('a')),
            ),
            keyPress(
              ~modifiers=modifiersControl,
              Key.Character(Uchar.of_char('b')),
            ),
          ]),
        ),
      );
    });
    test("explicitShiftKeyNeeded=false", ({expect, _}) => {
      let result = defaultParseImplicitShiftKey("A");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(
              ~modifiers=modifiersShift,
              Key.Character(Uchar.of_char('a')),
            ),
          ]),
        ),
      );

      let result = defaultParseImplicitShiftKey("Ab");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(
              ~modifiers=modifiersShift,
              Key.Character(Uchar.of_char('a')),
            ),
            keyPress(Key.Character(Uchar.of_char('b'))),
          ]),
        ),
      );
    });
    test("#2980 - separate modifier keys", ({expect, _}) => {
      let result = defaultParse("<c-w>r");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(
              ~modifiers=modifiersControl,
              Key.Character(Uchar.of_char('w')),
            ),
            keyPress(Key.Character(Uchar.of_char('r'))),
          ]),
        ),
      );
    });
    //    test("keyup", ({expect, _}) => {
    //      let result = defaultParse("!a");
    //      expect.equal(
    //        result,
    //        Ok(Sequence([Keyup(Keycode(1, Modifiers.none))])),
    //      );
    //
    //      let result = defaultParse("a!a");
    //      expect.equal(
    //        result,
    //        Ok(
    //          Sequence([
    //            1, Modifiers.none)),
    //            Keyup(Keycode(1, Modifiers.none)),
    //          ]),
    //        ),
    //      );
    //
    //      let result = defaultParse("a !Ctrl+a");
    //      expect.equal(
    //        result,
    //        Ok(
    //          Sequence([
    //            1, Modifiers.none)),
    //            Keyup(Keycode(1, modifiersControl)),
    //          ]),
    //        ),
    //      );
    //
    //      let result = defaultParse("a !<C-A>");
    //      expect.equal(
    //        result,
    //        Ok(
    //          Sequence([
    //            1, Modifiers.none)),
    //            Keyup(Keycode(1, modifiersControl)),
    //          ]),
    //        ),
    //      );
    //    });
  });
  describe("utf8", ({test, _}) => {
    test("#3599 - unicode characters parse correctly", ({expect, _}) => {
      let result = defaultParse("œ");
      expect.equal(
        result,
        Ok(Sequence([keyPress(Key.Character(Uchar.of_int(339)))])),
      );
    });

    test(
      "#3599 - unicode characters w/ vim-style modifiers parse correctly",
      ({expect, _}) => {
      let result = defaultParse("ctrl+œ");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(
              ~modifiers=modifiersControl,
              Key.Character(Uchar.of_int(339)),
            ),
          ]),
        ),
      );
    });
  });
});
