/*
 * NodeProcess.re
 *
 * Simple convenience wrapper around a node process
 */

open Oni_Core;

type t = {
  pid: int,
  stdout: in_channel,
  stderr: in_channel,
  stdin: out_channel,
};

let start = (~args=[], ~env=[], setup: Setup.t, scriptPath: string) => {
  let (pstdin, stdin) = Unix.pipe();
  let (stdout, pstdout) = Unix.pipe();
  let (stderr, pstderr) = Unix.pipe();

  let args = [setup.nodePath, scriptPath, ...args] |> Array.of_list;

  let env = env |> Array.of_list |> Array.append(Unix.environment());

  let pid =
    Unix.create_process_env(
      setup.nodePath,
      args,
      env,
      pstdin,
      pstdout,
      pstderr,
    );

  Unix.close(pstdout);
  Unix.close(pstderr);
  Unix.close(pstdin);

  let in_channel = Unix.in_channel_of_descr(stdout);
  let err_channel = Unix.in_channel_of_descr(stderr);
  let out_channel = Unix.out_channel_of_descr(stdin);

  let logPrefix =
    "["
    ++ string_of_int(pid)
    ++ "|"
    ++ Rench.Path.filename(scriptPath)
    ++ "]: ";

  let _ =
    Thread.create(
      () => {
        let shouldClose = ref(false);
        while (! shouldClose^) {
          Thread.wait_read(stderr);

          switch (input_line(err_channel)) {
          | exception End_of_file => shouldClose := true
          | v => Log.isDebugLoggingEnabled() ? Log.debug(() => logPrefix ++ v) : ()
          };
        };
      },
      (),
    );

  {pid, stdin: out_channel, stdout: in_channel, stderr: err_channel};
};
