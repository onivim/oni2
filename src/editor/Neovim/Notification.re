/*
 * NeovimProtocol.re
 *
 * Wrapper on-top of the NeovimApi primitives to hide the Msgpack details.
 */

/* open Oni_Core; */
/* open Rench; */

open Types;

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

type t =
  | Redraw
  | ModeChanged(string)
  | BufferLines(BufferLinesNotification.t)
  | CursorMoved(AutoCommandContext.t)
  | CommandlineShow(Commandline.t)
  | CommandlineHide(Commandline.t)
  | Ignored;

type commandlineInput = {input: string};

module M = Msgpck;

let showCommandline = args => {
  /*
     Structure of a cmdline_show response
     [cmdline_show, [[[attr, inputStr]], position, firstCharacter, indentAmount, index, level]]

     TODO: handle the attributes being sent
   */
  switch (args) {
  | [
      M.List(c),
      M.Int(position),
      M.String(firstC),
      M.String(_prompt),
      M.Int(index),
      M.Int(level),
    ] =>
    let {input} =
      List.fold_left(
        (accum, v) =>
          switch (v) {
          | M.List([M.Int(_), M.String(chunk)]) =>
            print_endline("v: " ++ chunk);
            {input: accum.input ++ chunk};
          | _ => accum
          },
        {input: ""},
        c,
      );
    CommandlineShow({
      content: input,
      firstC,
      position,
      index,
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
    index: 0,
    level: 0,
    show: false,
  });
};

let parseRedraw = (msgs: list(Msgpck.t)) => {
  let p = (msg: Msgpck.t) => {
    switch (msg) {
    | M.List([M.String("cmdline_show"), M.List(msgs)]) =>
      showCommandline(msgs)
    | M.List([M.String("cmdline_hide"), M.List(msgs)]) =>
      hideCommandline(msgs)
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
