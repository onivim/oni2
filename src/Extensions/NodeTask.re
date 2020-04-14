/*

 * NodeTask.re
 */

open Oni_Core;

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
  let {pid, stdout, _}: NodeProcess.t =
    NodeProcess.start(~args, setup, scriptPath);

  let shouldClose = ref(false);
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

  promise;
};
