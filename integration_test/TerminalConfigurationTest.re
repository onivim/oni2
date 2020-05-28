open Oni_Model;
open Oni_IntegrationTestLib;

let configuration =
  Some(
    {|
{
  "terminal.integrated.shell.windows": "win-shell",
  "terminal.integrated.shell.osx": "osx-shell",
  "terminal.integrated.shell.linux": "linux-shell",

  "terminal.integrated.shellArgs.windows": ["win-arg"],
  "terminal.integrated.shellArgs.osx": ["osx-arg"],
  "terminal.integrated.shellArgs.linux": ["linux-arg"]
}
|},
  );

let expectedShellCmd =
  switch (Revery.Environment.os) {
  | Windows => "win-shell"
  | Mac => "osx-shell"
  | Linux => "linux-shell"
  | _ => failwith("Unsupported os")
  };

let expectedShellArgs =
  switch (Revery.Environment.os) {
  | Windows => ["win-arg"]
  | Mac => ["osx-arg"]
  | Linux => ["linux-arg"]
  | _ => failwith("Unsupported os")
  };

runTest(
  ~configuration,
  ~name="TerminalConfigurationTest",
  (dispatch, wait, _) => {
    // Wait until the extension is activated
    // Give some time for the exthost to start
    wait(
      ~timeout=30.0,
      ~name="Validate the 'oni-dev' extension gets activated",
      (state: State.t) =>
      List.exists(
        id => id == "oni-dev-extension",
        state.extensions.activatedIds,
      )
    );

    // Spin up a terminal - no command is specified, so
    // it'll pull the default from configuration
    dispatch(
      Actions.Terminal(
        Feature_Terminal.Command(
          NewTerminal({cmd: None, splitDirection: Vertical, closeOnExit: false}),
        ),
      ),
    );

    let expectedTerminalExists = (state: State.t) => {
      state.terminals
      |> Feature_Terminal.toList
      |> List.exists((terminal: Feature_Terminal.terminal) => {
           terminal.cmd == expectedShellCmd
           && terminal.arguments == expectedShellArgs
         });
    };

    wait(
      ~name=
        "Terminal with proper commands and argument should've been created",
      (state: State.t) => {
      state |> expectedTerminalExists
    });
  },
);
