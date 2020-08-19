type modifier =
  | Control
  | Shift
  | Alt
  | Meta;

type activation =
  | Keyup
  | Keydown;

type keyMatcher = (activation, Key.t, list(modifier));

type t =
  | Sequence(list(keyMatcher))
  | AllKeysReleased;
