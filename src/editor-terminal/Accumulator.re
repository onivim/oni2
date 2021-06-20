module Accumulator = {
  type t('state, 'item) = {
    initialState: 'state,
    currentState: 'state,
    shouldMerge: ('state, 'item) => bool,
    merge: ('state, 'item) => 'state,
    flush: 'state => unit,
  };

  let create =
      (
        ~initialState: 'state,
        ~shouldMerge: ('state, 'item) => bool,
        ~merge: ('state, 'item) => 'state,
        ~flush: 'state => unit,
      ) => {
    currentState: initialState,
    initialState,
    shouldMerge,
    merge,
    flush,
  };

  let add = (item: 'item, accumulator: t('state, 'item)) => {
    let {shouldMerge, currentState, merge, flush, initialState} = accumulator;
    if (shouldMerge(currentState, item)) {
      let newState = merge(currentState, item);
      {...accumulator, currentState: newState};
    } else {
      flush(accumulator.currentState);
      let newState = merge(initialState, item);
      {...accumulator, currentState: newState};
    };
  };

  let flush = accumulator => {
    accumulator.flush(accumulator.currentState);
  };
};

module BackgroundColorAccumulator = {
  type cells = {
    startColumn: int,
    endColumn: int,
    color: Skia.Color.t,
  };

  type state = option(cells);

  type item = {
    color: Skia.Color.t,
    column: int,
  };

  type t = Accumulator.t(state, item);

  let initialState = None;

  let shouldMerge = (state, item) =>
    switch (state) {
    | None => true
    | Some({color, _}: cells) => color == item.color
    };

  let merge = (state: state, item: item) =>
    switch (state) {
    | None =>
      Some({
        startColumn: item.column,
        endColumn: item.column + 1,
        color: item.color,
      })
    | Some({color, startColumn, _}) =>
      Some({startColumn, endColumn: item.column + 1, color})
    };

  let create = draw => {
    let flush: state => unit =
      state => {
        switch (state) {
        | None => ()
        | Some(state) =>
          draw(state.startColumn, state.endColumn, state.color)
        };
      };

    Accumulator.create(~initialState, ~shouldMerge, ~merge, ~flush);
  };
};

module TextAccumulator = {
  type cells = {
    startColumn: int,
    color: Skia.Color.t,
    buffer: Buffer.t,
  };

  type state = option(cells);

  type item = {
    column: int,
    uchar: Uchar.t,
    color: Skia.Color.t,
  };

  let isValidUchar = uchar => {
    let codeInt = Uchar.to_int(uchar);

    codeInt !== 0 && codeInt <= 0x10FFF;
  };

  let shouldMerge = (state: state, item: item) =>
    switch (state) {
    | None => true
    | Some({color, _}) => color == item.color && isValidUchar(item.uchar)
    };

  let merge: (state, item) => state =
    (state: state, item: item) =>
      switch (state) {
      | None when !isValidUchar(item.uchar) => None
      | None =>
        let buffer = Buffer.create(16);
        Buffer.add_utf_8_uchar(buffer, item.uchar);
        Some({startColumn: item.column, color: item.color, buffer});
      | Some({startColumn, buffer, color}) =>
        Buffer.add_utf_8_uchar(buffer, item.uchar);
        Some({startColumn, buffer, color});
      };

  let initialState = None;

  let create = draw => {
    let flush: state => unit =
      state => {
        switch (state) {
        | None => ()
        | Some(state) => draw(state.startColumn, state.buffer, state.color)
        };
      };

    Accumulator.create(~initialState, ~shouldMerge, ~merge, ~flush);
  };
};
