type direction =
  | Forwards
  | Backwards;

type t = {
  line: int,
  index: int,
};

let find:
  (
    ~buffer: EditorBuffer.t,
    ~line: int,
    ~index: int,
    ~direction: direction,
    ~current: Uchar.t,
    ~destination: Uchar.t
  ) =>
  option(t);
