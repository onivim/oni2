/*
 * NeovimProtocol.re
 *
 * Wrapper on-top of the NeovimApi primitives to hide the Msgpack details.
 */

/* open Oni_Core; */
/* open Rench; */

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
    | ("redraw", M.List(msgs)) => parseRedraw(msgs)
    | _ => [Ignored]
    };

  msgs |> List.filter(m => m !== Ignored);
};

let show = (n: t) => {
  switch (n) {
  | Redraw => "redraw"
  | ModeChanged(s) => "mode changed: " ++ s
  | BufferLines(_) => "buffer lines"
  | _ => "unknown"
  };
};
