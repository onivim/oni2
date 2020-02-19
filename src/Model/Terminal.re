/*
 * Terminal.re
 *
 * State for active terminals
 */

type t = {
  id: int,
  cmd: string,
  rows: int,
  columns: int,
};
