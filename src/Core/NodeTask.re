/*
 * NodeTask.re
 */

open Utility;
exception TaskFailed;
module Log = (val Kernel.Log.withNamespace("Oni2.Core.NodeTask"));

let run = (~name="Anonymous", ~args=[], ~setup: Setup.t, script: string) => {
  Log.info("Starting task: " ++ name);
  let (promise, resolver) = Lwt.task();

  let scriptPath = Setup.getNodeScriptPath(~script, setup);
  Log.info("Using node path: " ++ scriptPath);

  let result = {
    open Base.Result.Let_syntax;

    let%bind stdoutPipe = Luv.Pipe.init();
    let%bind stderrPipe = Luv.Pipe.init();

    let on_exit = (_, ~exit_status, ~term_signal as _) =>
      if (exit_status == 0L) {
        Log.info("Task completed successfully: " ++ name);
      } else {
        Lwt.wakeup_exn(resolver, TaskFailed);
      };

    let%bind _: Luv.Process.t =
      LuvEx.Process.spawn(
        ~on_exit,
        ~detached=true,
        ~redirect=[
          Luv.Process.to_parent_pipe(
            ~fd=Luv.Process.stdout,
            ~parent_pipe=stdoutPipe,
            (),
          ),
          Luv.Process.to_parent_pipe(
            ~fd=Luv.Process.stderr,
            ~parent_pipe=stderrPipe,
            (),
          ),
        ],
        setup.nodePath,
        [setup.nodePath, scriptPath, ...args],
      );

    let buffers = ref([]);

    // If process was created successfully, we'll read from stdout
    let () =
      Luv.Stream.read_start(
        stdoutPipe,
        fun
        | Error(`EOF) => {
            Log.info("Got EOF on stdout");
            Luv.Handle.close(stdoutPipe, ignore);
            let allOutput =
              buffers^
              |> List.rev
              |> List.map(Luv.Buffer.to_string)
              |> String.concat("");
            Log.infof(m => m("Got output: %s", allOutput));
            Lwt.wakeup(resolver, allOutput);
          }
        | Error(msg) => Log.error(Luv.Error.strerror(msg))
        | Ok(buffer) => buffers := [buffer, ...buffers^],
      );

    let () =
      Luv.Stream.read_start(
        stderrPipe,
        fun
        | Error(`EOF) => {
            Log.info("Got EOF on stderr");
            Luv.Handle.close(stderrPipe, ignore);
          }
        | Error(msg) => Log.error(Luv.Error.strerror(msg))
        | Ok(buffer) => Log.error(Luv.Buffer.to_string(buffer)),
      );
    Ok();
  };

  switch (result) {
  | Ok(_) => ()
  | Error(err) =>
    Log.error(Luv.Error.strerror(err));
    Lwt.wakeup_exn(resolver, TaskFailed);
  };
  promise;
};
