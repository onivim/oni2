/**
 * Mode.re
 *
 * Onivim2 - specific modes (a superset of libvim modes)
 */

type t =
  // Vim modes
  | Normal
  | Insert
  | Visual({range: VisualRange.t})
  | Select({range: VisualRange.t})
  | Replace
  | Operator
  | CommandLine
  // Additional modes
  | TerminalInsert
  | TerminalNormal
  | TerminalVisual({range: VisualRange.t});

let toString =
  fun
  | Insert => "Insert"
  | Normal => "Normal"
  | Visual({range}) =>
    switch (range.mode) {
    | Character => "Visual Character"
    | Line => "Visual Line"
    | Block => "Visual Block"
    | None => "Visual"
    }
  | Select({range}) =>
    switch (range.mode) {
    | Character => "Select Character"
    | Line => "Select Line"
    | Block => "Select Block"
    | None => "Select"
    }
  | Replace => "Replace"
  | Operator => "Operator"
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
