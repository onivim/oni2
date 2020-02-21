open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="TerminalSetPidTitle", (dispatch, wait, _) => {
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

  // Spin up a terminal
  dispatch(
    Actions.Terminal(
      Feature_Terminal.NewTerminal({splitDirection: Vertical}),
    ),
  );

  let getTerminalOpt = (state: State.t) => {
    state.terminals
    |> Feature_Terminal.toList
    |> (list => List.nth_opt(list, 0));
  };

  wait(~name="Terminal should now be in state.", (state: State.t) => {
    state |> getTerminalOpt != None
  });

  wait(
    ~timeout=30.0,
    ~name="Validate terminal started and set the pid / title",
    (state: State.t) =>
    switch (getTerminalOpt(state)) {
    | None => failwith("Terminal should be in state!")
    | Some({id, pid, title, _}) =>
      let logOpt = str =>
        str
        |> Option.map(str => "Some: " ++ str)
        |> Option.value(~default="None");

      prerr_endline(
        Printf.sprintf(
          "Checking for terminal - id: %d pid: %s title: %s",
          id,
          pid |> Option.map(string_of_int) |> logOpt,
          title |> logOpt,
        ),
      );

      pid != None && title != None;
    }
  );
});
