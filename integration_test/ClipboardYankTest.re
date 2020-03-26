open Oni_Model;
open Oni_IntegrationTestLib;

let optEqual = (sOpt, s) => {
  switch (sOpt) {
  | None => false
  | Some(v) => String.equal(v, s)
  };
};

let printOpt = sOpt =>
  switch (sOpt) {
  | None => "[None]"
  | Some(v) => "Some(" ++ v ++ ")"
  };

runTest(~name="ClipboardYankTest", (dispatch, wait, runEffects) => {
  wait(
    ~name="Set configuration to always yank to clipboard", (state: State.t) => {
    let configuration = state.configuration;
    dispatch(
      ConfigurationSet({
        ...configuration,
        default: {
          ...configuration.default,
          vimUseSystemClipboard: {
            yank: true,
            delete: false,
            paste: false,
          },
        },
      }),
    );
    runEffects();
    true;
  });

  dispatch(KeyboardInput("i"));
  wait(~name="Mode switches to insert", (state: State.t) =>
    state.vimMode == Vim.Types.Insert
  );

  dispatch(KeyboardInput("a"));
  dispatch(KeyboardInput("b"));
  dispatch(KeyboardInput("c"));
  dispatch(KeyboardInput("<esc>"));
  runEffects();

  wait(~name="Mode switches back to normal", (state: State.t) =>
    state.vimMode == Vim.Types.Normal
  );

  setClipboard(None);

  /* Validate yank w/ no register sets clipboard */
  dispatch(KeyboardInput("y"));
  dispatch(KeyboardInput("y"));
  runEffects();

  wait(~name="Yank 1 is sent to clipboard", _ => {
    print_endline("CLIPBOARD: " ++ printOpt(getClipboard()));
    optEqual(getClipboard(), "abc");
  });

  setClipboard(None);
  /* Validate yank w/ a register sets clipboard */
  dispatch(KeyboardInput("\""));
  dispatch(KeyboardInput("a"));
  dispatch(KeyboardInput("y"));
  dispatch(KeyboardInput("y"));
  runEffects();

  wait(~name="Yank 2 is sent to clipboard", _ => {
    print_endline("CLIPBOARD: " ++ printOpt(getClipboard()));
    optEqual(getClipboard(), "abc");
  });

  wait(
    ~name="Set configuration to not yank to clipboard by default",
    (state: State.t) => {
    let configuration = state.configuration;
    dispatch(
      ConfigurationSet({
        ...configuration,
        default: {
          ...configuration.default,
          vimUseSystemClipboard: {
            yank: false,
            delete: false,
            paste: false,
          },
        },
      }),
    );
    runEffects();
    true;
  });

  setClipboard(None);
  /* Validate yank w/ no register doesn't set clipboard */
  dispatch(KeyboardInput("y"));
  dispatch(KeyboardInput("y"));
  runEffects();

  wait(~name="Yank 3 is NOT sent to clipboard", _ => {
    print_endline("CLIPBOARD: " ++ printOpt(getClipboard()));
    getClipboard() == None;
  });

  setClipboard(None);
  /* Validate yank w/ a register doesn't set clipboard */
  dispatch(KeyboardInput("\""));
  dispatch(KeyboardInput("a"));
  dispatch(KeyboardInput("y"));
  dispatch(KeyboardInput("y"));
  runEffects();

  wait(~name="Yank 4 w/ register 'a' is NOT sent to clipboard", _ => {
    print_endline("CLIPBOARD: " ++ printOpt(getClipboard()));
    getClipboard() == None;
  });
  setClipboard(None);
  /* Validate yank w/ + register sets clipboard */
  dispatch(KeyboardInput("\""));
  dispatch(KeyboardInput("+"));
  dispatch(KeyboardInput("y"));
  dispatch(KeyboardInput("y"));
  runEffects();

  wait(~name="Yank 4 w/ register '+' is still sent to clipboard", _ => {
    print_endline("CLIPBOARD: " ++ printOpt(getClipboard()));
    optEqual(getClipboard(), "abc");
  });
});
