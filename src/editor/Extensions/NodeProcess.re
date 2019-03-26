/*
 * NodeProcess.re
 *
 * Simple convenience wrapper around a node process
 */

open Oni_Core;

type t = {
  pid: int,
  stdout: in_channel,
  stdin: out_channel,
};

let start = (~args=[], ~env=[], setup: Setup.t, scriptPath: string) => {
  let (pstdin, stdin) = Unix.pipe();
  let (stdout, pstdout) = Unix.pipe();

  let args = [setup.nodePath, scriptPath, ...args] |> Array.of_list;

  let env = env |> Array.of_list |> Array.append(Unix.environment());

  let pid =
    Unix.create_process_env(
      setup.nodePath,
      args,
      env,
      pstdin,
      pstdout,
      Unix.stderr,
    );

  let in_channel = Unix.in_channel_of_descr(stdout);
  let out_channel = Unix.out_channel_of_descr(stdin);

  Unix.close(pstdout);
  Unix.close(pstdin);

  {pid, stdin: out_channel, stdout: in_channel};
};
