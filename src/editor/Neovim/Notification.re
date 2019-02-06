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
  | Ignored;

module M = Msgpck;

let parseRedraw = (msgs: list(Msgpck.t)) => {
  let p = (msg: Msgpck.t) => {
    switch (msg) {
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
