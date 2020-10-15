open EditorCoreTypes;

type t =
  | Normal({cursor: BytePosition.t})
  | Insert({cursors: list(BytePosition.t)})
  | CommandLine
  | Replace({cursor: BytePosition.t})
  | Visual({range: VisualRange.t})
  | Operator({
      cursor: BytePosition.t,
      pending: Operator.pending,
    })
  | Select({range: VisualRange.t});

let show = (mode: t) => {
  switch (mode) {
  | Normal(_) => "Normal"
  | Visual(_) => "Visual"
  | CommandLine => "CommandLine"
  | Replace(_) => "Replace"
  | Operator(_) => "Operator"
  | Insert(_) => "Insert"
  | Select(_) => "Select"
  };
};

let cursors =
  fun
  | Normal({cursor}) => [cursor]
  | Insert({cursors}) => cursors
  | Replace({cursor}) => [cursor]
  | Visual({range}) => [range |> VisualRange.cursor]
  | Operator({cursor, _}) => [cursor]
  | Select({range}) => [range |> VisualRange.cursor]
  | CommandLine => [];

let current = () => {
  let nativeMode: Native.mode = Native.vimGetMode();

  let cursor = Cursor.get();
  switch (nativeMode) {
  | Native.Normal => Normal({cursor: cursor})
  | Native.Visual => Visual({range: VisualRange.current()})
  | Native.CommandLine => CommandLine
  | Native.Replace => Replace({cursor: cursor})
  | Native.Operator =>
    Operator({
      cursor,
      pending: Operator.get() |> Option.value(~default=Operator.default),
    })
  | Native.Insert => Insert({cursors: [cursor]})
  | Native.Select => Select({range: VisualRange.current()})
  };
};

let isVisual =
  fun
  | Visual(_) => true
  | _ => false;

let isSelect =
  fun
  | Select(_) => true
  | _ => false;

let isInsert =
  fun
  | Insert(_) => true
  | _ => false;

let isNormal =
  fun
  | Normal(_) => true
  | _ => false;

let isReplace =
  fun
  | Replace(_) => true
  | _ => false;

let isOperatorPending =
  fun
  | Operator(_) => true
  | _ => false;
