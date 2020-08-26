type t =
  | Normal
  | Insert
  | CommandLine
  | Replace
  | Visual({range: VisualRange.t})
  | Operator({pending: Operator.pending})
  | Select({range: VisualRange.t});

let show = (mode: t) => {
  switch (mode) {
  | Normal => "Normal"
  | Visual(_) => "Visual"
  | CommandLine => "CommandLine"
  | Replace => "Replace"
  | Operator(_) => "Operator"
  | Insert => "Insert"
  | Select(_) => "Select"
  };
};

let current = () => {
  let nativeMode: Native.mode = Native.vimGetMode();

  switch (nativeMode) {
  | Native.Normal => Normal
  | Native.Visual =>
    Visual({
      range:
        VisualRange.create(
          ~range=Visual.getRange(),
          ~visualType=Visual.getType(),
          (),
        ),
    })
  | Native.CommandLine => CommandLine
  | Native.Replace => Replace
  | Native.Operator =>
    Operator({
      pending: Operator.get() |> Option.value(~default=Operator.default),
    })
  | Native.Insert => Insert
  | Native.Select =>
    Select({
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
