open EditorCoreTypes;

type t =
  | Normal({cursor: BytePosition.t})
  | Insert({cursors: list(BytePosition.t)})
  | CommandLine
  | Replace({cursor: BytePosition.t})
  | Visual({
      cursor: BytePosition.t,
      range: VisualRange.t,
    })
  | Operator({
      cursor: BytePosition.t,
      pending: Operator.pending,
    })
  | Select({
      cursor: BytePosition.t,
      range: VisualRange.t,
    });

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
  | Visual({cursor, _}) => [cursor]
  | Operator({cursor, _}) => [cursor]
  | Select({cursor, _}) => [cursor]
  | CommandLine => [];

let current = () => {
  let nativeMode: Native.mode = Native.vimGetMode();

  let cursor = Cursor.get();
  switch (nativeMode) {
  | Native.Normal => Normal({cursor: cursor})
  | Native.Visual =>
    Visual({
      cursor,
      range:
        VisualRange.create(
          ~range=Visual.getRange(),
          ~visualType=Visual.getType(),
          (),
        ),
    })
  | Native.CommandLine => CommandLine
  | Native.Replace => Replace({cursor: cursor})
  | Native.Operator =>
    Operator({
      cursor,
      pending: Operator.get() |> Option.value(~default=Operator.default),
    })
  | Native.Insert => Insert({cursors: [cursor]})
  | Native.Select =>
    Select({
      cursor,
      range:
        VisualRange.create(
          ~range=Visual.getRange(),
          ~visualType=Visual.getType(),
          (),
        ),
    })
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
