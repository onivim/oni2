/*
 * NodeTask.rei
 */

exception TaskFailed;

let run:
  (~name: string=?, ~args: list(string)=?, ~setup: Setup.t, string) =>
  Lwt.t(string);
