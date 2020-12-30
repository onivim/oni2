open TestFramework;
open EditorInput;

// let getKeycode =
//   fun
//   | Key.Character('a') => Some(1)
//   | Key.Character('A') => Some(1)
//   | Key.Character('b') => Some(2)
//   | Key.Character('0') => Some(50)
//   | Key.Character('9') => Some(59)
//   | Key.Character('`') => Some(60)
//   | Key.Character('-') => Some(61)
//   | Key.Character('=') => Some(62)
//   | Key.Character('[') => Some(63)
//   | Key.Character(']') => Some(64)
//   | Key.Character('\\') => Some(65)
//   | Key.Character(';') => Some(66)
//   | Key.Character('\'') => Some(67)
//   | Key.Character(',') => Some(68)
//   | Key.Character('.') => Some(69)
//   | Key.Character('/') => Some(70)
//   | Key.Tab => Some(98)
//   | Key.Escape => Some(99)
//   | Key.Up => Some(100)
//   | Key.Left => Some(101)
//   | Key.Right => Some(102)
//   | Key.Down => Some(103)
//   | Key.PageUp => Some(104)
//   | Key.PageDown => Some(105)
//   | Key.End => Some(106)
//   | Key.Home => Some(107)
//   | Key.Return => Some(108)
//   | Key.Space => Some(109)
//   | Key.Backspace => Some(110)
//   | Key.Delete => Some(111)
//   | Key.Pause => Some(112)
//   | Key.CapsLock => Some(113)
//   | Key.Insert => Some(114)
//   | Key.Function(0) => Some(115)
//   | Key.Function(12) => Some(133)
//   | Key.Function(19) => Some(134)
//   | Key.NumpadDigit(0) => Some(135)
//   | Key.NumpadDigit(9) => Some(144)
//   | NumpadMultiply => Some(145)
//   | NumpadAdd => Some(146)
//   | NumpadSeparator => Some(147)
//   | NumpadSubtract => Some(148)
//   | NumpadDecimal => Some(149)
//   | NumpadDivide => Some(150)
//   | _ => None;

// let getScancode = getKeycode;

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
        ("a", keyPress(Key.Character('a'))),
        ("A", keyPress(Key.Character('a'))), // Because implicit shift is false
        ("0", keyPress(Key.Character('0'))),
        ("9", keyPress(Key.Character('9'))),
        ("`", keyPress(Key.Character('`'))),
        ("-", keyPress(Key.Character('-'))),
        ("=", keyPress(Key.Character('='))),
        ("[", keyPress(Key.Character('['))),
        ("]", keyPress(Key.Character(']'))),
        ("\\", keyPress(Key.Character('\\'))),
        (";", keyPress(Key.Character(';'))),
        ("'", keyPress(Key.Character('\''))),
        (",", keyPress(Key.Character(','))),
        (".", keyPress(Key.Character('.'))),
        ("/", keyPress(Key.Character('/'))),
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
        // TODO: Split into Uchar?
        (":", keyPress(Key.String(":"))),
        ("~", keyPress(Key.String("~"))),
        ("_", keyPress(Key.Character('_'))),
        ("+", keyPress(Key.Character('+'))),
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
      expect.equal(result, Ok(Sequence([keyPress(Key.Character('a'))])));

      let result = defaultParse("b");
      expect.equal(result, Ok(Sequence([keyPress(Key.Character('b'))])));

      let result = defaultParse("esc");
      expect.equal(result, Ok(Sequence([keyPress(Key.Escape)])));
    });
    test("all keys released", ({expect, _}) => {
      let result = defaultParse("<RELEASE>");
      expect.equal(result, Ok(AllKeysReleased));
    });
    test("vim bindings", ({expect, _}) => {
      let result = defaultParse("<a>");
      expect.equal(result, Ok(Sequence([keyPress(Key.Character('a'))])));

      let result = defaultParse("<c-a>");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(~modifiers=modifiersControl, Key.Character('a')),
          ]),
        ),
      );

      let result = defaultParse("<S-a>");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(~modifiers=modifiersShift, Key.Character('a')),
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
    });
    test("vscode bindings", ({expect, _}) => {
      let result = defaultParse("Ctrl+a");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(~modifiers=modifiersControl, Key.Character('a')),
          ]),
        ),
      );

      let result = defaultParse("ctrl+a");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(~modifiers=modifiersControl, Key.Character('a')),
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
            keyPress(Key.Character('a')),
            keyPress(Key.Character('b')),
          ]),
        ),
      );

      let result = defaultParse("a b");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(Key.Character('a')),
            keyPress(Key.Character('b')),
          ]),
        ),
      );

      let result = defaultParse("<a>b");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(Key.Character('a')),
            keyPress(Key.Character('b')),
          ]),
        ),
      );
      let result = defaultParse("<a><b>");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(Key.Character('a')),
            keyPress(Key.Character('b')),
          ]),
        ),
      );

      let result = defaultParse("<c-a> Ctrl+b");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(~modifiers=modifiersControl, Key.Character('a')),
            keyPress(~modifiers=modifiersControl, Key.Character('b')),
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
            keyPress(~modifiers=modifiersShift, Key.Character('a')),
          ]),
        ),
      );

      let result = defaultParseImplicitShiftKey("Ab");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(~modifiers=modifiersShift, Key.Character('a')),
            keyPress(Key.Character('b')),
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
  })
});
