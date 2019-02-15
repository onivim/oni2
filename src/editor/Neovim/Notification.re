/*
 * NeovimProtocol.re
 *
 * Wrapper on-top of the NeovimApi primitives to hide the Msgpack details.
 */

/* open Oni_Core; */
/* open Rench; */

open Types;

module Core = Oni_Core;

module BufferLinesNotification = {
  type t = {
    changedTick: int,
    firstLine: int,
    lastLine: int,
    lines: list(string),
  };

  let make = (changedTick, firstLine, lastLine, lineData) => {
    let f = s => {
      switch (s) {
      | Msgpck.String(v) => v
      | _ => ""
      };
    };

    let lines = List.map(f, lineData);

    let ret: t = {changedTick, firstLine, lastLine, lines};
    ret;
  };
};

module BufferEnterNotification = {
  type t = {
    context: AutoCommandContext.t,
    bufferId: int,
  };
};

type t =
  | Redraw
  | ModeChanged(string)
  | BufferLines(BufferLinesNotification.t)
  | BufferEnter(BufferEnterNotification.t)
  | CursorMoved(AutoCommandContext.t)
  | CommandlineShow(Commandline.t)
  | CommandlineHide(Commandline.t)
  | WildmenuShow(Wildmenu.t)
  | WildmenuHide(Wildmenu.t)
  | WildmenuSelected(int)
  | TablineUpdate(Tabline.tabs)
  | Ignored;

type commandlineInput = {input: string};

module M = Msgpck;

let showCommandline = args => {
  /*
     Structure of a cmdline_show response
     [cmdline_show, [[[attr, inputStr]], position, firstCharacter, prompt, indentAmount, level]]

     TODO: handle the attributes being sent
   */
  switch (args) {
  | [
      M.List(c),
      M.Int(position),
      M.String(firstC),
      M.String(prompt),
      M.Int(indent),
      M.Int(level),
    ] =>
    let {input} =
      List.fold_left(
        (accum, v) =>
          switch (v) {
          | M.List([M.Int(_), M.String(chunk)]) => {
              input: accum.input ++ chunk,
            }
          | _ => accum
          },
        {input: ""},
        c,
      );
    CommandlineShow({
      content: input,
      prompt,
      firstC,
      position,
      indent,
      level,
      show: true,
    });
  | _ => Ignored
  };
};

let hideCommandline = _msgs => {
  CommandlineHide({
    content: "",
    firstC: "",
    position: 0,
    indent: 0,
    prompt: "",
    level: 0,
    show: false,
  });
};

let showWildmenu = (args: list(M.t)) => {
  /* [[wildmenu_show, [[list items]]] */
  switch (args) {
  | [M.List(i)] =>
    let items =
      List.fold_left(
        (accum, item) =>
          switch (item) {
          | M.String(i) => [i, ...accum]
          | _ => accum
          },
        [],
        i,
      )
      /* reorder the list due to the spreading above adding items the wrong way round */
      |> List.rev;
    WildmenuShow({items, selected: 0, show: true});
  | _ => Ignored
  };
};

let hideWildmenu = _msgs => {
  WildmenuHide({items: [], show: false, selected: 0});
};

let updateWildmenu = selected => {
  /* [wildmenu_select, [0]] */
  switch (selected) {
  | M.Int(s) => WildmenuSelected(s)
  | _ => Ignored
  };
};

/*
   Msgpck has a special type for Maps, which returns a tuple
   consisting of the key and the value, so we pattern
   match on the name and value and use those to construct
   the record type we want
 */
let parseTabMap = map => {
  Core.(
    Tabline.(
      List.fold_left(
        (accum, item) =>
          switch (item) {
          | (M.String("name"), M.String(value)) => {...accum, name: value}
          | (M.String("tab"), value) =>
            let tab =
              switch (Utility.convertNeovimExtType(value)) {
              | Some((_, id)) => id
              | None => 0
              };
            {...accum, tab};
          | _ => accum
          },
        {name: "", tab: 0},
        map,
      )
    )
  );
};

let parseTablineUpdate = msgs => {
  /*
     The structure of a tabline update is -
     [tabline_update, [(2 "\001"), [{tab: (2 "\001"), name: string}]]],
   */
  switch (msgs) {
  | [_view, Msgpck.List(tabs)] =>
    let allTabs =
      List.fold_left(
        (accum, tab) =>
          switch (tab) {
          | M.Map(map) =>
            let map = parseTabMap(map);
            [Tabline.{tab: map.tab, name: map.name}, ...accum];
          | _ => accum
          },
        [],
        tabs,
      );
    TablineUpdate(allTabs);
  | _ => Ignored
  };
};

let parseRedraw = (msgs: list(Msgpck.t)) => {
  let p = (msg: Msgpck.t) => {
    switch (msg) {
    | M.List([M.String("cmdline_show"), M.List(msgs)]) =>
      showCommandline(msgs)
    | M.List([M.String("cmdline_hide"), M.List(msgs)]) =>
      hideCommandline(msgs)
    | M.List([M.String("wildmenu_show"), M.List(msgs)]) =>
      showWildmenu(msgs)
    | M.List([M.String("wildmenu_select"), M.List([selected])]) =>
      updateWildmenu(selected)
    | M.List([M.String("wildmenu_hide"), M.List(msgs)]) =>
      hideWildmenu(msgs)
    | M.List([M.String("tabline_update"), M.List(msgs)]) =>
      parseTablineUpdate(msgs)
    | M.List([
        M.String("mode_change"),
        M.List([M.String(mode), M.Int(_style)]),
      ]) =>
      ModeChanged(mode)
    | _ => Ignored
    };
  };

  msgs |> List.map(p);
};

exception InvalidAutoCommandContext;

let parseAutoCommand = (autocmd: string, args: list(Msgpck.t)) => {
  let context =
    switch (args) {
    | [M.Int(activeBufferId), M.Int(cursorLine), M.Int(cursorColumn)] =>
      Types.AutoCommandContext.create(
        ~activeBufferId,
        ~cursorLine=OneBasedIndex(cursorLine),
        ~cursorColumn=OneBasedIndex(cursorColumn),
        (),
      )
    | _ => raise(InvalidAutoCommandContext)
    };

  switch (autocmd) {
  | "BufEnter" =>
    switch (args) {
    | [M.Int(bufferId), _, _] => BufferEnter({context, bufferId})
    | _ => Ignored
    }
  | "CursorMoved" => CursorMoved(context)
  | "CursorMovedI" => CursorMoved(context)
  | _ => Ignored
  };
};

let parse = (t: string, msg: Msgpck.t) => {
  let msgs =
    switch (t, msg) {
    | (
        "nvim_buf_lines_event",
        M.List([
          _,
          M.Int(changedTick),
          M.Int(firstLine),
          M.Int(lastLine),
          M.List(lineData),
          _,
        ]),
      ) => [
        BufferLines(
          BufferLinesNotification.make(
            changedTick,
            firstLine,
            lastLine,
            lineData,
          ),
        ),
      ]
    | (
        "oni_plugin_notify",
        M.List([
          M.List([M.String("autocmd"), M.String(autocmd), M.List(args)]),
        ]),
      ) =>
      let result = parseAutoCommand(autocmd, args);
      [result];
    | ("redraw", M.List(msgs)) => parseRedraw(msgs)
    | _ => [Ignored]
    };

  msgs |> List.filter(m => m !== Ignored);
};

let show = (n: t) => {
  switch (n) {
  | Redraw => "redraw"
  | ModeChanged(s) => "mode changed: " ++ s
  | CursorMoved(c) => "cursor moved: " ++ AutoCommandContext.show(c)
  | BufferLines(_) => "buffer lines"
  | _ => "unknown"
  };
};
