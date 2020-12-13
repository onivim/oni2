let keyToSdlName =
  EditorInput.Key.(
    {
      fun
      | Escape => "Escape"
      | Return => "Return"
      | Up => "Up"
      | Down => "Down"
      | Left => "Left"
      | Right => "Right"
      | Tab => "Tab"
      | PageUp => "PageUp"
      | PageDown => "PageDown"
      | Delete => "Delete"
      | Pause => "Pause"
      | Home => "Home"
      | End => "End"
      | Backspace => "Backspace"
      | CapsLock => "CapsLock"
      | Insert => "Insert"
      | Function(digit) => "F" ++ string_of_int(digit)
      | Space => "Space"
      | NumpadDigit(digit) => "Keypad " ++ string_of_int(digit)
      | NumpadMultiply => "Keypad *"
      | NumpadAdd => "Keypad +"
      | NumpadSeparator => "Keypad "
      | NumpadSubtract => "Keypad -"
      | NumpadDivide => "Keypad //"
      | NumpadDecimal => "Keypad ."
      | Character(c) => String.make(1, c) |> String.uppercase_ascii;
    }
  );

let getKeycode = inputKey => {
  inputKey
  |> keyToSdlName
  |> Sdl2.Keycode.ofName
  |> (
    fun
    | 0 => None
    | x => Some(x)
  );
};

let getScancode = inputKey => {
  inputKey
  |> keyToSdlName
  |> Sdl2.Scancode.ofName
  |> (
    fun
    | 0 => None
    | x => Some(x)
  );
};
