/*
 * NodeProcess.re
 *
 * Simple convenience wrapper around a node process
 */

type t = {
  pid: int,
  stdout: in_channel,
  stdin: out_channel,
};

let start = (setup: Setup.t, scriptPath: string) => {
  let (pstdin, stdin) = Unix.pipe();
  let (stdout, pstdout) = Unix.pipe();

  let pid =
    Unix.create_process_env(
      setup.nodePath,
      [|setup.nodePath, scriptPath|],
      Unix.environment(),
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
