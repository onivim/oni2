/**
 * Mode.re
 *
 * Onivim2 - specific modes (a superset of libvim modes)
 */

type t =
  // Vim modes
  | Normal
  | Insert
  | Visual
  | Select
  | Replace
  | Operator
  | CommandLine
  // Additional modes
  | TerminalInsert
  | TerminalNormal
  | TerminalVisual;

let toString =
  fun
  | Insert => "Insert"
  | Normal => "Normal"
  | Visual => "Visual"
  | Select => "Select"
  | Replace => "Replace"
  | Operator => "Operator"
  | CommandLine => "Command Line"
  | TerminalInsert => "Terminal Insert"
  | TerminalNormal => "Terminal Normal"
  | TerminalVisual => "Terminal Visual";
