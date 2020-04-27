/*
 * NodeTask.re
 */

open Oni_Core;
open Oni_Core.Utility;
exception TaskFailed;
module Log = (val Log.withNamespace("Oni2.Extensions.NodeTask"));

let run =
    (
      ~name="Anonymous",
      ~scheduler=Scheduler.mainThread,
      ~args=[],
      ~setup: Setup.t,
      script: string,
    ) => {
  Log.info("Starting task: " ++ name);
  let (promise, resolver) = Lwt.task();

  let scriptPath = Setup.getNodeScriptPath(~script, setup);
  Log.info("Using node path: " ++ scriptPath);

  let processResult =
    Luv.Pipe.init()
    |> ResultEx.tap(pipe => Luv.Pipe.open_(pipe, Luv.File.stdout))
    |> ResultEx.flatMap(parent_pipe => {
         let on_exit = (_, ~exit_status, ~term_signal as _) =>
           if (exit_status == 0L) {
             Log.info("Task completed successfully: " ++ name);
             Lwt.wakeup(resolver, ());
           } else {
             Lwt.wakeup_exn(resolver, TaskFailed);
           };

         Luv.Process.spawn(
           ~on_exit,
           setup.nodePath,
           [setup.nodePath, scriptPath, ...args],
         );
       })
    |> Result.map_error(Luv.Error.strerror);

  processResult
  |> Result.iter_error(err => {
       Log.error(err);
       Lwt.wakeup_exn(resolver, TaskFailed);
     });

  /*let shouldClose = ref(false);
    let _readStdout: Thread.t =
      ThreadHelper.create(
        ~name="NodeTask.stdout",
        () => {
          let running = ref(true);
          while (running^) {
            try({
              let str = input_line(stdout);
              Scheduler.run(() => Log.info(str), scheduler);
            }) {
            | End_of_file => running := false
            };
          };
        },
        (),
      );

    let _waitThread: Thread.t =
      ThreadHelper.create(
        ~name="NodeTask.wait",
        () => {
          let (_code, status: Unix.process_status) = Unix.waitpid([], pid);

          switch (status) {
          | WEXITED(0) =>
            Log.info("Task completed successfully: " ++ name);
            Lwt.wakeup(resolver, ());
          | _ =>
            Log.warn("Task failed: " ++ name);
            Lwt.wakeup_exn(resolver, TaskFailed);
          };

          shouldClose := true;
        },
        (),
      );

    promise;*/
  promise;
};
