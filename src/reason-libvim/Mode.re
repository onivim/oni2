open EditorCoreTypes;

type t =
  | Normal({cursor: BytePosition.t})
  | Insert({cursors: list(BytePosition.t)})
  | CommandLine({
      text: string,
      commandCursor: ByteIndex.t,
      commandType: Types.cmdlineType,
      cursor: BytePosition.t,
    })
  | Replace({cursor: BytePosition.t})
  | Visual(VisualRange.t)
  | Operator({
      cursor: BytePosition.t,
      pending: Operator.pending,
    })
  | Select({ranges: list(VisualRange.t)});

let show = (mode: t) => {
  switch (mode) {
  | Normal(_) => "Normal"
  | Visual(_) => "Visual"
  | CommandLine(_) => "CommandLine"
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
  | Visual(range) => [range |> VisualRange.cursor]
  | Operator({cursor, _}) => [cursor]
  | Select({ranges}) => ranges |> List.map(VisualRange.cursor)
  | CommandLine({cursor, _}) => [cursor];

let ranges =
  fun
  | Normal(_) => []
  | Insert(_) => []
  | Replace(_) => []
  | Visual(range) => [range]
  | Operator(_) => []
  | Select({ranges}) => ranges
  | CommandLine(_) => [];

let current = () => {
  let nativeMode: Native.mode = Native.vimGetMode();

  let cursor = Cursor.get();
  switch (nativeMode) {
  | Native.Normal => Normal({cursor: cursor})
  | Native.Visual => Visual(VisualRange.current())
  | Native.CommandLine =>
    let commandCursor = Native.vimCommandLineGetPosition() |> ByteIndex.ofInt;
    let commandType = Native.vimCommandLineGetType();
    let text = Native.vimCommandLineGetText() |> Option.value(~default="");
    CommandLine({cursor, commandCursor, commandType, text});
  | Native.Replace => Replace({cursor: cursor})
  | Native.Operator =>
    Operator({
      cursor,
      pending: Operator.get() |> Option.value(~default=Operator.default),
    })
  | Native.Insert => Insert({cursors: [cursor]})
  | Native.Select => Select({ranges: [VisualRange.current()]})
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

let isInsertOrSelect = mode => isInsert(mode) || isSelect(mode);

let isCommandLine =
  fun
  | CommandLine(_) => true
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

module Internal = {
  let ensureNormal = () =>
    if (!isNormal(current())) {
      Native.vimKey("<ESC>");
      Native.vimKey("<ESC>");
    };

  let ensureInsert = () =>
    if (!isInsert(current())) {
      ensureNormal();
      Native.vimKey("i");
    };

  let ensureVisual = visualType =>
    if (!isVisual(current())) {
      ensureNormal();
      Types.(
        switch (visualType) {
        | Block => Native.vimKey("<C-v>")
        | Line => Native.vimKey("V")
        | Character
        | None => Native.vimKey("v")
        }
      );
    };

  let ensureSelect = visualType =>
    if (!isSelect(current())) {
      ensureVisual(visualType);
      Native.vimKey("<c-g>");
    };
};

let trySet = newMode => {
  switch (newMode) {
  | Normal({cursor}) =>
    Internal.ensureNormal();
    Cursor.set(cursor);
  | Visual(range) =>
    Internal.ensureVisual(range.visualType);
    Visual.set(
      ~visualType=range.visualType,
      ~start=range.anchor,
      ~cursor=range.cursor,
    );
  | Select({ranges}) =>
    switch (ranges) {
    | [range, ..._] =>
      Internal.ensureSelect(range.visualType);
      Visual.set(
        ~visualType=range.visualType,
        ~start=range.anchor,
        ~cursor=range.cursor,
      );
    | [] => ()
    }
  | Insert(_) => Internal.ensureInsert()
  // These modes cannot be explicitly transitioned to currently
  | Operator(_)
  | Replace(_)
  | CommandLine(_) => ()
  };

  current();
};
