open Oni_Core;

[@deriving show]
type t =
  | Character([@opaque] Uchar.t)
  | Function(int)
  | NumpadDigit(int)
  | Escape
  | Down
  | Up
  | Left
  | Right
  | Tab
  | PageUp
  | PageDown
  | Return
  | Space
  | Delete
  | Pause
  | Home
  | End
  | Backspace
  | CapsLock
  | Insert
  | NumpadMultiply
  | NumpadAdd
  | NumpadSeparator
  | NumpadSubtract
  | NumpadDecimal
  | NumpadDivide
  | LeftControl
  | RightControl;

let toString =
  fun
  | Character(c) => ZedBundled.make(1, c)
  | Function(digit) => Printf.sprintf("F%d", digit)
  | NumpadDigit(digit) => Printf.sprintf("Numpad%d", digit)
  | Escape => "Escape"
  | Down => "Down"
  | Up => "Up"
  | Left => "Left"
  | Right => "Right"
  | Tab => "Tab"
  | PageUp => "PageUp"
  | PageDown => "PageDown"
  | Return => "Return"
  | Space => "Space"
  | Delete => "Delete"
  | Pause => "PauseBreak"
  | Home => "Home"
  | End => "End"
  | Backspace => "Backspace"
  | CapsLock => "CapsLock"
  | Insert => "Insert"
  | NumpadMultiply => "NumpadMultiply"
  | NumpadAdd => "NumpadAdd"
  | NumpadSeparator => "NumpadSeparator"
  | NumpadSubtract => "NumpadSubtract"
  | NumpadDecimal => "NumpadDecimal"
  | NumpadDivide => "NumpadDivide"
  | LeftControl => "LeftControl"
  | RightControl => "RightControl";
