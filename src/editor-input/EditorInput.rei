module Key: {
  type t =
    | Character(char)
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
    | NumpadDivide;

  let toString: t => string;
};

module Modifiers: {
  [@deriving show]
  type t = {
    control: bool,
    alt: bool,
    altGr: bool,
    shift: bool,
    meta: bool,
  };

  let none: t;

  let equals: (t, t) => bool;
};

module Matcher: {
  type keyMatcher =
    | Scancode(int, Modifiers.t)
    | Keycode(int, Modifiers.t);

  type keyPress =
    | Keydown(keyMatcher)
    | Keyup(keyMatcher);

  type t =
    | Sequence(list(keyPress))
    | AllKeysReleased;

  let parse:
    (
      ~getKeycode: Key.t => option(int),
      ~getScancode: Key.t => option(int),
      string
    ) =>
    result(t, string);
};

module KeyPress: {
  [@deriving show]
  type t = {
    scancode: int,
    keycode: int,
    modifiers: Modifiers.t,
  };

  let toString:
    // The name of the 'meta' key. Defaults to "Meta".
    (~meta: string=?, ~keyCodeToString: int => string, t) => string;
};

module type Input = {
  type command;
  type context;

  type t;

  type uniqueId;

  let addBinding: (Matcher.t, context => bool, command, t) => (t, uniqueId);

  let addMapping:
    (Matcher.t, context => bool, list(KeyPress.t), t) => (t, uniqueId);

  type effect =
    // The `Execute` effect means that a key-sequence associated with `command`
    // has been completed, and the `command` should now be executed.
    | Execute(command)
    // The `Text` effect occurs when an unhandled `text` input event occurs.
    | Text(string)
    // The `Unhandled` effect occurs when an unhandled `keyDown` input event occurs.
    // This can happen if there is no binding associated with a key.
    | Unhandled(KeyPress.t)
    // RemapRecursionLimitHit is produced if there is a recursive loop
    // in remappings such that we hit the max limit.
    | RemapRecursionLimitHit;

  let keyDown: (~context: context, ~key: KeyPress.t, t) => (t, list(effect));
  let text: (~text: string, t) => (t, list(effect));
  let keyUp: (~context: context, ~key: KeyPress.t, t) => (t, list(effect));

  /**
  [isPending(bindings)] returns true if there is a potential
  keybinding pending, false otherwise
  */
  let isPending: t => bool;

  let count: t => int;

  let concat: (t, t) => t;

  let empty: t;
};

module Make:
  (Context: {
     type command;
     type context;
   }) =>
   Input with type command = Context.command and type context = Context.context;
