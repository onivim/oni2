/*
 * NodeTask.re
 */

open Utility;
exception TaskFailed;
module Log = (val Kernel.Log.withNamespace("Oni2.Core.NodeTask"));

module Internal = {
  let getFilteredEnvironment = () => {
    // Filter out some keys that aren't required
    // for running node tasks. Primarily needed for Windows -
    // sometimes the environment can grow too large for node/libuv.
    let keysToExclude =
      ["OCAMLPATH", "MAN_PATH", "CAML_LD_LIBRARY_PATH"]
      |> Kernel.StringSet.of_list;

    let env =
      Unix.environment()
      |> Array.to_list
      |> List.fold_left(
           (acc, curr) => {
             switch (String.split_on_char('=', curr)) {
             | [] => acc
             | [_] => acc
             | [key, ...values] =>
               if (!Kernel.StringSet.mem(key, keysToExclude)) {
                 let v = String.concat("=", values);

                 [(key, v), ...acc];
               } else {
                 acc;
               }
             }
           },
           [],
         );
    env;
  };
};

let run = (~name="Anonymous", ~args=[], ~setup: Setup.t, script: string) => {
  Log.info("Starting task: " ++ name);
  let (promise, resolver) = Lwt.task();

  let scriptPath = Setup.getNodeScriptPath(~script, setup);
  Log.info("Using node path: " ++ scriptPath);

  let result = {
    open Base.Result.Let_syntax;

    let%bind stdoutPipe = Luv.Pipe.init();
    let%bind stderrPipe = Luv.Pipe.init();

    let buffers = ref([]);
    let hasClosedStdout = ref(false);
    let hasExited = ref(false);

    let success = () => {
      let allOutput =
        buffers^ |> List.rev_map(Luv.Buffer.to_string) |> String.concat("");
      Log.debugf(m => m("Got output: %s", allOutput));
      Lwt.wakeup(resolver, allOutput);
    };

    let tryToFinish = () =>
      if (hasExited^ && hasClosedStdout^) {
        success();
      } else if (! hasExited^) {
        Log.info("Trying to finish, but process hasn't exited yet.");
      } else if (! hasClosedStdout^) {
        Log.info("Trying to finish, but have not received EOF yet.");
      };

    let on_exit = (_, ~exit_status, ~term_signal as _) => {
      Log.infof(m => m("on_exit called with exit_status: %Ld", exit_status));
      if (exit_status == 0L) {
        hasExited := true;
        tryToFinish();
        Log.info("Task completed successfully: " ++ name);
      } else {
        Lwt.wakeup_exn(resolver, TaskFailed);
      };
    };

    let%bind _: Luv.Process.t =
      LuvEx.Process.spawn(
        ~on_exit,
        ~environment=Internal.getFilteredEnvironment(),
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

    // If process was created successfully, we'll read from stdout
    let () =
      Luv.Stream.read_start(
        stdoutPipe,
        fun
        | Error(`EOF) => {
            hasClosedStdout := true;
            Log.info("Got EOF on stdout");
            Luv.Handle.close(stdoutPipe, ignore);
            tryToFinish();
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
