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

module PhysicalKey: {
  [@deriving show]
  type t = {
    scancode: int,
    keycode: int,
    modifiers: Modifiers.t,
  };
};

module SpecialKey: {
  [@deriving show]
  type t =
    // Leader key defined by 'vim.leader' or `let mapleader = "<space>"` in VimL
    | Leader
    // Special key <Plug> used by VimL plugins
    // No physical key associated with it, but useful for scoping remappings.
    | Plug
    // Special key <Nop> used by Vim as no-op
    | Nop;
  // TODO;
  // | SNR;
};

module KeyPress: {
  [@deriving show]
  type t =
    | PhysicalKey(PhysicalKey.t)
    | SpecialKey(SpecialKey.t);

  let toString:
    // The name of the 'meta' key. Defaults to "Meta".
    (~meta: string=?, ~keyCodeToString: int => string, t) => string;

  let physicalKey:
    (~keycode: int, ~scancode: int, ~modifiers: Modifiers.t) => t;

  let specialKey: SpecialKey.t => t;

  let toPhysicalKey: t => option(PhysicalKey.t);

  let parse:
    // When [explicitShiftKeyNeeded] is [true]:
    // - Both 's' and 'S' would get resolved as 's'
    // In other words, 'S' requires a 'Shift+' modifier
    // (VScode style parsing)
    // When [explicitShiftKeyNeeded] is [false]:
    // - 's' would get resolved as 's', 'S' would get resolved as 'Shift+s'
    // (Vim style parsing)
    (
      ~explicitShiftKeyNeeded: bool,
      ~getKeycode: Key.t => option(int),
      ~getScancode: Key.t => option(int),
      string
    ) =>
    result(list(t), string);
};

module Matcher: {
  type t =
    | Sequence(list(KeyPress.t))
    | AllKeysReleased;

  let parse:
    // When [explicitShiftKeyNeeded] is [true]:
    // - Both 's' and 'S' would get resolved as 's'
    // In other words, 'S' requires a 'Shift+' modifier
    // (VScode style parsing)
    // When [explicitShiftKeyNeeded] is [false]:
    // - 's' would get resolved as 's', 'S' would get resolved as 'Shift+s'
    // (Vim style parsing)
    (
      ~explicitShiftKeyNeeded: bool,
      ~getKeycode: Key.t => option(int),
      ~getScancode: Key.t => option(int),
      string
    ) =>
    result(t, string);
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

  let keyDown:
    (
      ~leaderKey: option(PhysicalKey.t)=?,
      ~context: context,
      ~key: KeyPress.t,
      t
    ) =>
    (t, list(effect));
  let text: (~text: string, t) => (t, list(effect));
  let keyUp:
    (
      ~leaderKey: option(PhysicalKey.t)=?,
      ~context: context,
      ~key: KeyPress.t,
      t
    ) =>
    (t, list(effect));

  let remove: (uniqueId, t) => t;

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
