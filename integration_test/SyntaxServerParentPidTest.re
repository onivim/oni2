open Oni_IntegrationTestLib;

let createDummyProcess = () => {
  let cmd = "bash";
  try({
    let (inchannel, outchannel) = Unix.open_process(cmd);
    let pid = Unix.process_pid((inchannel, outchannel));

    let stop = () => {
      prerr_endline(Printf.sprintf("Killing process with pid: %d", pid));
      Unix.close_process((inchannel, outchannel));
    };

    prerr_endline(Printf.sprintf("Created dummy process with pid: %d", pid));
    (pid, stop);
  }) {
  | ex =>
    Printexc.to_string(ex) |> prerr_endline;
    exit(2);
  };
};

let (pid, stop) = createDummyProcess();

SyntaxServerTest.run(
  ~name="SyntaxServerParentPidTest",
  // Lie to the syntax server, and pretend the dummy process
  // that we control is the parent process.
  ~parentPid=string_of_int(pid),
  ({hasExited, wait, isConnected, _}) => {
    wait(~name="Connected", isConnected);

    // Kill the process...
    switch (stop()) {
    | WEXITED(_) => prerr_endline("WEXITED")
    | WSIGNALED(_) => prerr_endline("WSIGNALED")
    | WSTOPPED(_) => prerr_endline("WSTOPPED")
    };

    // This should close automatically, since there is no parentPid
    // present...
    wait(~name="Closed", ~timeout=30.0, ()
      // TODO: This should be exit code 0,
      // but there is a bug on Windows preventing this - the 'wait_pid' behavior.
      // hasExited() == Some(0)
      => Option.is_some(hasExited()));
  },
);
