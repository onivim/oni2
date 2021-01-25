/**
 * Mode.re
 *
 * Onivim2 - specific modes (a superset of libvim modes)
 */
open EditorCoreTypes;

type t =
  // Vim modes
  | Normal({cursor: BytePosition.t})
  | Insert({cursors: list(BytePosition.t)})
  | Visual({
      cursor: BytePosition.t,
      range: VisualRange.t,
    })
  | Select({ranges: list(VisualRange.t)})
  | Replace({cursor: BytePosition.t})
  | Operator({pending: Vim.Operator.pending})
  | CommandLine
  // Additional modes
  | TerminalInsert
  | TerminalNormal
  | TerminalVisual({range: VisualRange.t});

let toString =
  fun
  | Insert(_) => "Insert"
  | Normal(_) => "Normal"
  | Visual({range, _}) =>
    switch (range.mode) {
    | Character => "Visual Character"
    | Line => "Visual Line"
    | Block => "Visual Block"
    | None => "Visual"
    }
  | Select({ranges, _}) =>
    switch (ranges) {
    | [range, ..._] =>
      switch (range.mode) {
      | Character => "Select Character"
      | Line => "Select Line"
      | Block => "Select Block"
      | None => "Select"
      }
    | [] => "Select"
    }
  | Replace(_) => "Replace"
  | Operator({pending}) => Vim.Operator.toString(pending)
  | CommandLine => "Command Line"
  | TerminalInsert => "Terminal Insert"
  | TerminalNormal => "Terminal Normal"
  | TerminalVisual({range}) =>
    switch (range.mode) {
    | Character => "Terminal Visual Character"
    | Line => "Terminal Visual Line"
    | Block => "Terminal Visual Block"
    | None => "Terminal Visual"
    };
