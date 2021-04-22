module Key: {
  type t =
    | Character(Uchar.t)
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

  let toString: t => string;
};

module Modifiers: {
  [@deriving show]
  type t = {
    control: bool,
    alt: bool,
    altGr: bool,
    shift: bool,
    super: bool,
  };

  let none: t;

  let equals: (t, t) => bool;
};

module PhysicalKey: {
  [@deriving show]
  type t = {
    key: Key.t,
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
    // The name of the 'super' key. Defaults to "Super".
    (~super: string=?, ~keyToString: Key.t => string=?, t) => string;

  let physicalKey: (~key: Key.t, ~modifiers: Modifiers.t) => t;

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
    (~explicitShiftKeyNeeded: bool, string) => result(list(t), string);
};

module KeyCandidate: {
  // A KeyCandidate is a list of potential key-presses that could match.
  // For example, on a US keyboard, a `Shift+=` key could be interpreted multiple ways:
  // - `<S-=>`
  // - `+`
  // We should be able to handle bindings of either type, but the burden is on the consumer
  // to provide the potential match candidates.
  [@deriving show]
  type t;

  // [ofKeyPress(keyPress)] creates a candidate of a single key press
  let ofKeyPress: KeyPress.t => t;

  // [ofList(keyPresses)] creates a candidate from multiple key presses
  let ofList: list(KeyPress.t) => t;

  // [toList(candidate)] returns a list of key presses associate with the candidate
  let toList: t => list(KeyPress.t);
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
    (~explicitShiftKeyNeeded: bool, string) => result(t, string);

  let toString: t => string;
};

module type Input = {
  type command;
  type context;

  type t;

  type uniqueId;

  let addBinding: (Matcher.t, context => bool, command, t) => (t, uniqueId);

  let addMapping:
    (
      ~allowRecursive: bool,
      Matcher.t,
      context => bool,
      list(KeyPress.t),
      t
    ) =>
    (t, uniqueId);

  // Turn off all bindings, as if no bindings are defined
  let disable: t => t;

  // Turn on binding handling
  let enable: t => t;

  type effect =
    // The `Execute` effect means that a key-sequence associated with `command`
    // has been completed, and the `command` should now be executed.
    | Execute(command)
    // The `Text` effect occurs when an unhandled `text` input event occurs.
    | Text(string)
    // The `Unhandled` effect occurs when an unhandled `keyDown` input event occurs.
    // This can happen if there is no binding associated with a key.
    | Unhandled({
        key: KeyCandidate.t,
        isProducedByRemap: bool,
      })
    // RemapRecursionLimitHit is produced if there is a recursive loop
    // in remappings such that we hit the max limit.
    | RemapRecursionLimitHit;

  let keyDown:
    (
      ~leaderKey: option(PhysicalKey.t)=?,
      ~context: context,
      ~scancode: int,
      ~key: KeyCandidate.t,
      t
    ) =>
    (t, list(effect));

  let text: (~text: string, t) => (t, list(effect));

  let keyUp:
    (
      ~leaderKey: option(PhysicalKey.t)=?,
      ~context: context,
      ~scancode: int,
      t
    ) =>
    (t, list(effect));

  let timeout: (~context: context, t) => (t, list(effect));

  // [candidates] returns a list of available matcher / command
  // candidates, based on the current context and input state.
  let candidates:
    (~leaderKey: option(PhysicalKey.t), ~context: context, t) =>
    list((Matcher.t, command));

  // [consumedKeys(model)] returns a list of keys
  // that are currently consumed by the state machine.
  let consumedKeys: t => list(KeyCandidate.t);

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
