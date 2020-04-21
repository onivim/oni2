open Oni_IntegrationTestLib;

open Oni_IntegrationTestLib;

let win32Command = {
  switch (Sys.getenv_opt("COMSPEC")) {
  | None => "cmd.exe"
  | Some(cmd) => cmd
  };
};

let createDummyProcess = () => {
  let cmd = Sys.win32 ? win32Command : "bash";
  try({
    let (inchannel, outchannel) = Unix.open_process(cmd);
    let pid = Unix.process_pid((inchannel, outchannel));

    let stop = () => Unix.close_process((inchannel, outchannel));

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
    // TODO: Why doesn't initial connection pass on Windows?
    // Is it a PID issue, or an issue with waitpid on Windows?
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
