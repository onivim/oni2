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
    |> ResultEx.flatMap(parent_pipe => {
         let on_exit = (_, ~exit_status, ~term_signal as _) =>
           if (exit_status == 0L) {
             Log.info("Task completed successfully: " ++ name);
             Lwt.wakeup(resolver, ());
           } else {
             Lwt.wakeup_exn(resolver, TaskFailed);
           };

         let process =
           Luv.Process.spawn(
             ~on_exit,
             ~detached=true,
             ~windows_hide=true,
             ~windows_hide_console=true,
             ~windows_hide_gui=true,
             ~redirect=[
               Luv.Process.to_parent_pipe(
                 ~fd=Luv.Process.stdout,
                 ~parent_pipe,
                 (),
               ),
             ],
             setup.nodePath,
             [setup.nodePath, scriptPath, ...args],
           );

         // If process was created successfully, we'll read from stdout
         process
         |> Result.iter(_ => {
              Luv.Stream.read_start(
                parent_pipe,
                fun
                | Error(`EOF) => {
                    Log.info("Done!");
                    Luv.Handle.close(parent_pipe, ignore);
                  }
                | Error(msg) => Log.error(Luv.Error.strerror(msg))
                | Ok(buffer) => Log.info(Luv.Buffer.to_string(buffer)),
              )
            });
         process;
       })
    |> Result.map_error(Luv.Error.strerror);

  processResult
  |> Result.iter_error(err => {
       Log.error(err);
       Lwt.wakeup_exn(resolver, TaskFailed);
     });

  promise;
};
