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
        ("a", Keydown(Keycode(keyPress(1)))),
        ("A", Keydown(Keycode(keyPress(1)))),
        ("0", Keydown(Keycode(keyPress(50)))),
        ("9", Keydown(Keycode(keyPress(59)))),
        ("`", Keydown(Keycode(keyPress(60)))),
        ("-", Keydown(Keycode(keyPress(61)))),
        ("=", Keydown(Keycode(keyPress(62)))),
        ("[", Keydown(Keycode(keyPress(63)))),
        ("]", Keydown(Keycode(keyPress(64)))),
        ("\\", Keydown(Keycode(keyPress(65)))),
        (";", Keydown(Keycode(keyPress(66)))),
        ("'", Keydown(Keycode(keyPress(67)))),
        (",", Keydown(Keycode(keyPress(68)))),
        (".", Keydown(Keycode(keyPress(69)))),
        ("/", Keydown(Keycode(keyPress(70)))),
        ("tab", Keydown(Keycode(keyPress(98)))),
        ("ESC", Keydown(Keycode(keyPress(99)))),
        ("up", Keydown(Keycode(keyPress(100)))),
        ("left", Keydown(Keycode(keyPress(101)))),
        ("right", Keydown(Keycode(keyPress(102)))),
        ("down", Keydown(Keycode(keyPress(103)))),
        ("PageUp", Keydown(Keycode(keyPress(104)))),
        ("pagedown", Keydown(Keycode(keyPress(105)))),
        ("end", Keydown(Keycode(keyPress(106)))),
        ("home", Keydown(Keycode(keyPress(107)))),
        ("enter", Keydown(Keycode(keyPress(108)))),
        ("cr", Keydown(Keycode(keyPress(108)))),
        ("escape", Keydown(Keycode(keyPress(99)))),
        ("space", Keydown(Keycode(keyPress(109)))),
        ("bs", Keydown(Keycode(keyPress(110)))),
        ("backspace", Keydown(Keycode(keyPress(110)))),
        ("del", Keydown(Keycode(keyPress(111)))),
        ("delete", Keydown(Keycode(keyPress(111)))),
        ("pause", Keydown(Keycode(keyPress(112)))),
        ("capslock", Keydown(Keycode(keyPress(113)))),
        ("insert", Keydown(Keycode(keyPress(114)))),
        ("f0", Keydown(Keycode(keyPress(115)))),
        ("f19", Keydown(Keycode(keyPress(134)))),
        ("numpad0", Keydown(Keycode(keyPress(135)))),
        ("numpad9", Keydown(Keycode(keyPress(144)))),
        ("numpad_multiply", Keydown(Keycode(keyPress(145)))),
        ("numpad_add", Keydown(Keycode(keyPress(146)))),
        ("numpad_separator", Keydown(Keycode(keyPress(147)))),
        ("numpad_subtract", Keydown(Keycode(keyPress(148)))),
        ("numpad_decimal", Keydown(Keycode(keyPress(149)))),
        ("numpad_divide", Keydown(Keycode(keyPress(150)))),
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
        Ok(Sequence([Keydown(Keycode(keyPress(1)))])),
      );

      let result = defaultParse("b");
      expect.equal(
        result,
        Ok(Sequence([Keydown(Keycode(keyPress(2)))])),
      );

      let result = defaultParse("c");
      expect.equal(Result.is_error(result), true);

      let result = defaultParse("esc");
      expect.equal(
        result,
        Ok(Sequence([Keydown(Keycode(keyPress(99)))])),
      );
    });
    test("all keys released", ({expect, _}) => {
      let result = defaultParse("<RELEASE>");
      expect.equal(result, Ok(AllKeysReleased));
    });
    test("vim bindings", ({expect, _}) => {
      let result = defaultParse("<a>");
      expect.equal(
        result,
        Ok(Sequence([Keydown(Keycode(keyPress(1)))])),
      );

      let result = defaultParse("<c-a>");
      expect.equal(
        result,
        Ok(
          Sequence([
            Keydown(Keycode(keyPress(~modifiers=modifiersControl, 1))),
          ]),
        ),
      );

      let result = defaultParse("<S-a>");
      expect.equal(
        result,
        Ok(
          Sequence([
            Keydown(Keycode(keyPress(~modifiers=modifiersShift, 1))),
          ]),
        ),
      );

      let result = defaultParse("<S-F12>");
      expect.equal(
        result,
        Ok(
          Sequence([
            Keydown(Keycode(keyPress(~modifiers=modifiersShift, 133))),
          ]),
        ),
      );
    });
    test("vscode bindings", ({expect, _}) => {
      let result = defaultParse("Ctrl+a");
      expect.equal(
        result,
        Ok(
          Sequence([
            Keydown(Keycode(keyPress(~modifiers=modifiersControl, 1))),
          ]),
        ),
      );

      let result = defaultParse("ctrl+a");
      expect.equal(
        result,
        Ok(
          Sequence([
            Keydown(Keycode(keyPress(~modifiers=modifiersControl, 1))),
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
            Keydown(Keycode(keyPress(1))),
            Keydown(Keycode(keyPress(2))),
          ]),
        ),
      );

      let result = defaultParse("a b");
      expect.equal(
        result,
        Ok(
          Sequence([
            Keydown(Keycode(keyPress(1))),
            Keydown(Keycode(keyPress(2))),
          ]),
        ),
      );

      let result = defaultParse("<a>b");
      expect.equal(
        result,
        Ok(
          Sequence([
            Keydown(Keycode(keyPress(1))),
            Keydown(Keycode(keyPress(2))),
          ]),
        ),
      );
      let result = defaultParse("<a><b>");
      expect.equal(
        result,
        Ok(
          Sequence([
            Keydown(Keycode(keyPress(1))),
            Keydown(Keycode(keyPress(2))),
          ]),
        ),
      );

      let result = defaultParse("<c-a> Ctrl+b");
      expect.equal(
        result,
        Ok(
          Sequence([
            Keydown(Keycode(keyPress(~modifiers=modifiersControl, 1))),
            Keydown(Keycode(keyPress(~modifiers=modifiersControl, 2))),
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
    //            Keydown(Keycode(1, Modifiers.none)),
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
    //            Keydown(Keycode(1, Modifiers.none)),
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
    //            Keydown(Keycode(1, Modifiers.none)),
    //            Keyup(Keycode(1, modifiersControl)),
    //          ]),
    //        ),
    //      );
    //    });
  })
});
