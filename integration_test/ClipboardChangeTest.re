open Oni_Model;
open Oni_IntegrationTestLib;

// Regression test for #2694
runTest(~name="ClipboardChangeTest", (dispatch, wait, runEffects) => {
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

  dispatch(KeyboardInput({isText: true, input: "i"}));
  wait(~name="Mode switches to insert", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isInsert
  );

  dispatch(KeyboardInput({isText: true, input: "abc"}));
  dispatch(KeyboardInput({isText: false, input: "<esc>"}));
  runEffects();

  wait(~name="Mode switches back to normal", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  // Clear clipboard prior to test
  setClipboard(None);

  /* Validate yank w/ no register sets clipboard */
  dispatch(KeyboardInput({isText: true, input: "yy"}));
  runEffects();

  wait(~name="Yank 1 is sent to clipboard", _ => {
    getClipboard() == Some("abc\n")
  });

  setClipboard(None);

  /* Validate change does NOT set the clipbaord, with just 'yank' true
     should be treated as a delete (#2694) */
  dispatch(KeyboardInput({isText: true, input: "0"}));
  dispatch(KeyboardInput({isText: true, input: "c"}));
  dispatch(KeyboardInput({isText: true, input: "w"}));
  runEffects();

  wait(~name="Yank 2 is not sent to the clipboard", _ => {
    getClipboard() == None
  });

  // Undo change, bring back text
  dispatch(KeyboardInput({isText: false, input: "<esc>"}));
  dispatch(KeyboardInput({isText: false, input: "<esc>"}));
  dispatch(KeyboardInput({isText: true, input: "u"}));
  runEffects();

  wait(~name="Mode switches back to normal", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  wait(~name="Set configuration to yank on deletes", (state: State.t) => {
    let configuration = state.configuration;
    dispatch(
      ConfigurationSet({
        ...configuration,
        default: {
          ...configuration.default,
          vimUseSystemClipboard: {
            yank: false,
            delete: true,
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
  dispatch(KeyboardInput({isText: true, input: "0"}));
  dispatch(KeyboardInput({isText: true, input: "c"}));
  dispatch(KeyboardInput({isText: true, input: "w"}));
  runEffects();

  wait(~name="Change should now be sent to the clipboard", _ => {
    switch (getClipboard()) {
    | Some(cur) => prerr_endline(cur)
    | None => prerr_endline("(None)")
    };
    getClipboard() == Some("abc");
  });
});
