/*
 * NodeTask.rei
 */

open Oni_Core;

exception TaskFailed;

let run:
  (
    ~name: string=?,
    ~args: list(string)=?,
    ~setup: Setup.t,
    string
  ) =>
  Lwt.t(unit);
