/*
 * NodeTask.re
 */

open Oni_Core;

exception TaskFailed;

module Log = (val Log.withNamespace("Oni2.NodeTask"));

let run =
    (
      ~name="Anonymous",
      ~scheduler: (unit, unit) => unit,
      ~onMessage: string => unit,
      ~scriptPath: string,
      ~args=[],
      ~setup: Setup.t,
    ) => {
  let (promise, resolver) = Lwt.task();

  let {pid, stdout, stderr, _}: NodeProcess.t =
    NodeProcess.start(~args, setup, scriptPath);

  let shouldClose = ref(false);
  let _readStdout =
    Thread.create(
      () => {
        while (! shouldClose^) {
          let str = input_line(stdout);
          print_endline("STR: " ++ str);
        }
      },
      (),
    );

  let _readStderr =
    Thread.create(
      () => {
        while (! shouldClose^) {
          let str = input_line(stderr);
          print_endline("STR ERROR: " ++ str);
        }
      },
      (),
    );

  let _waitThread =
    Thread.create(
      () => {
        let (code, _status: Unix.process_status) = Unix.waitpid([], pid);

        if (code == 0) {
          Log.info("Task completed successfully: " ++ name);
          Lwt.wakeup(resolver, ());
        } else {
          Log.info("Task failed: " ++ name);
          Lwt.wakeup_exn(resolver, TaskFailed);
        };
        shouldClose := true;
      },
      (),
    );

  promise;
};
