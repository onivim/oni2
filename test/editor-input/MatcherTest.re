open TestFramework;
open EditorInput;

let getKeycode =
  fun
  | Key.Character('a') => Some(1)
  | Key.Character('b') => Some(2)
  | Key.Character('0') => Some(50)
  | Key.Character('9') => Some(59)
  | Key.Character('`') => Some(60)
  | Key.Character('-') => Some(61)
  | Key.Character('=') => Some(62)
  | Key.Character('[') => Some(63)
  | Key.Character(']') => Some(64)
  | Key.Character('\\') => Some(65)
  | Key.Character(';') => Some(66)
  | Key.Character('\'') => Some(67)
  | Key.Character(',') => Some(68)
  | Key.Character('.') => Some(69)
  | Key.Character('/') => Some(70)
  | Key.Tab => Some(98)
  | Key.Escape => Some(99)
  | Key.Up => Some(100)
  | Key.Left => Some(101)
  | Key.Right => Some(102)
  | Key.Down => Some(103)
  | Key.PageUp => Some(104)
  | Key.PageDown => Some(105)
  | Key.End => Some(106)
  | Key.Home => Some(107)
  | Key.Return => Some(108)
  | Key.Space => Some(109)
  | Key.Backspace => Some(110)
  | Key.Delete => Some(111)
  | Key.Pause => Some(112)
  | Key.CapsLock => Some(113)
  | Key.Insert => Some(114)
  | Key.Function(0) => Some(115)
  | Key.Function(12) => Some(133)
  | Key.Function(19) => Some(134)
  | Key.NumpadDigit(0) => Some(135)
  | Key.NumpadDigit(9) => Some(144)
  | NumpadMultiply => Some(145)
  | NumpadAdd => Some(146)
  | NumpadSeparator => Some(147)
  | NumpadSubtract => Some(148)
  | NumpadDecimal => Some(149)
  | NumpadDivide => Some(150)
  | _ => None;

let getScancode = getKeycode;

let defaultParse = Matcher.parse(~getKeycode, ~getScancode);

let modifiersControl = {...Modifiers.none, control: true};

let modifiersShift = {...Modifiers.none, shift: true};

let keyPress = (~modifiers=Modifiers.none, code) =>
  KeyPress.{keycode: code, scancode: code, modifiers};

describe("Matcher", ({describe, _}) => {
  describe("parser", ({test, _}) => {
    test("all keys", ({expect, _}) => {
      open Matcher;
      // Exercise full set of keys described here:
      // https://code.visualstudio.com/docs/getstarted/keybindings#_accepted-keys
      let cases = [
        ("a", keyPress(1)),
        ("A", keyPress(1)),
        ("0", keyPress(50)),
        ("9", keyPress(59)),
        ("`", keyPress(60)),
        ("-", keyPress(61)),
        ("=", keyPress(62)),
        ("[", keyPress(63)),
        ("]", keyPress(64)),
        ("\\", keyPress(65)),
        (";", keyPress(66)),
        ("'", keyPress(67)),
        (",", keyPress(68)),
        (".", keyPress(69)),
        ("/", keyPress(70)),
        ("tab", keyPress(98)),
        ("ESC", keyPress(99)),
        ("up", keyPress(100)),
        ("left", keyPress(101)),
        ("right", keyPress(102)),
        ("down", keyPress(103)),
        ("PageUp", keyPress(104)),
        ("pagedown", keyPress(105)),
        ("end", keyPress(106)),
        ("home", keyPress(107)),
        ("enter", keyPress(108)),
        ("cr", keyPress(108)),
        ("escape", keyPress(99)),
        ("space", keyPress(109)),
        ("bs", keyPress(110)),
        ("backspace", keyPress(110)),
        ("del", keyPress(111)),
        ("delete", keyPress(111)),
        ("pause", keyPress(112)),
        ("capslock", keyPress(113)),
        ("insert", keyPress(114)),
        ("f0", keyPress(115)),
        ("f19", keyPress(134)),
        ("numpad0", keyPress(135)),
        ("numpad9", keyPress(144)),
        ("numpad_multiply", keyPress(145)),
        ("numpad_add", keyPress(146)),
        ("numpad_separator", keyPress(147)),
        ("numpad_subtract", keyPress(148)),
        ("numpad_decimal", keyPress(149)),
        ("numpad_divide", keyPress(150)),
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
      expect.equal(result, Ok(Sequence([keyPress(1)])));

      let result = defaultParse("b");
      expect.equal(result, Ok(Sequence([keyPress(2)])));

      let result = defaultParse("c");
      expect.equal(Result.is_error(result), true);

      let result = defaultParse("esc");
      expect.equal(result, Ok(Sequence([keyPress(99)])));
    });
    test("all keys released", ({expect, _}) => {
      let result = defaultParse("<RELEASE>");
      expect.equal(result, Ok(AllKeysReleased));
    });
    test("vim bindings", ({expect, _}) => {
      let result = defaultParse("<a>");
      expect.equal(result, Ok(Sequence([keyPress(1)])));

      let result = defaultParse("<c-a>");
      expect.equal(
        result,
        Ok(Sequence([keyPress(~modifiers=modifiersControl, 1)])),
      );

      let result = defaultParse("<S-a>");
      expect.equal(
        result,
        Ok(Sequence([keyPress(~modifiers=modifiersShift, 1)])),
      );

      let result = defaultParse("<S-F12>");
      expect.equal(
        result,
        Ok(Sequence([keyPress(~modifiers=modifiersShift, 133)])),
      );
    });
    test("vscode bindings", ({expect, _}) => {
      let result = defaultParse("Ctrl+a");
      expect.equal(
        result,
        Ok(Sequence([keyPress(~modifiers=modifiersControl, 1)])),
      );

      let result = defaultParse("ctrl+a");
      expect.equal(
        result,
        Ok(Sequence([keyPress(~modifiers=modifiersControl, 1)])),
      );
    });
    test("binding list", ({expect, _}) => {
      let result = defaultParse("ab");
      expect.equal(result, Ok(Sequence([keyPress(1), keyPress(2)])));

      let result = defaultParse("a b");
      expect.equal(result, Ok(Sequence([keyPress(1), keyPress(2)])));

      let result = defaultParse("<a>b");
      expect.equal(result, Ok(Sequence([keyPress(1), keyPress(2)])));
      let result = defaultParse("<a><b>");
      expect.equal(result, Ok(Sequence([keyPress(1), keyPress(2)])));

      let result = defaultParse("<c-a> Ctrl+b");
      expect.equal(
        result,
        Ok(
          Sequence([
            keyPress(~modifiers=modifiersControl, 1),
            keyPress(~modifiers=modifiersControl, 2),
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
