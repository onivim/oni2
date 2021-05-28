/*
 * NodeTask.rei
 */

exception TaskFailed;

let run:
  (
    ~additionalEnvironment: list((string, string))=?,
    ~name: string=?,
    ~args: list(string)=?,
    ~setup: Setup.t,
    string
  ) =>
  Lwt.t(string);
