open Oni_Core;
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

runTest(~name="ClipboardYankTest", ({dispatch, wait, runEffects, _}) => {
  wait(~name="Set configuration to always yank to clipboard", _ => {
    let transformer =
      ConfigurationTransformer.setField(
        "vim.useSystemClipboard",
        `List([`String("yank")]),
      );
    dispatch(
      Configuration(Feature_Configuration.Testing.transform(transformer)),
    );
    runEffects();
    true;
  });

  wait(~name="Wait for configuration to update", (state: State.t) => {
    Feature_Configuration.Legacy.getValue(
      c => c.vimUseSystemClipboard,
      state.config,
    )
    == Feature_Configuration.LegacyConfigurationValues.{
         yank: true,
         delete: false,
         paste: false,
       }
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

  setClipboard(None);

  /* Validate yank w/ no register sets clipboard */
  dispatch(KeyboardInput({isText: true, input: "yy"}));
  runEffects();

  wait(~name="Yank 1 is sent to clipboard", _ => {
    print_endline("CLIPBOARD: " ++ printOpt(getClipboard()));
    optEqual(getClipboard(), "abc\n");
  });

  setClipboard(None);
  /* Validate yank w/ a register sets clipboard */
  dispatch(KeyboardInput({isText: true, input: "\"ayy"}));
  runEffects();

  wait(~name="Yank 2 is sent to clipboard", _ => {
    print_endline("CLIPBOARD: " ++ printOpt(getClipboard()));
    optEqual(getClipboard(), "abc\n");
  });
  wait(~name="Set configuration to not yank clipboard by default", _ => {
    let transformer =
      ConfigurationTransformer.setField(
        "vim.useSystemClipboard",
        `Bool(false),
      );
    dispatch(
      Configuration(Feature_Configuration.Testing.transform(transformer)),
    );
    runEffects();
    true;
  });

  wait(~name="Wait for configuration to update", (state: State.t) => {
    Feature_Configuration.Legacy.getValue(
      c => c.vimUseSystemClipboard,
      state.config,
    )
    == Feature_Configuration.LegacyConfigurationValues.{
         yank: false,
         delete: false,
         paste: false,
       }
  });

  setClipboard(None);
  /* Validate yank w/ no register doesn't set clipboard */
  dispatch(KeyboardInput({isText: true, input: "yy"}));
  runEffects();

  wait(~name="Yank 3 is NOT sent to clipboard", _ => {
    print_endline("CLIPBOARD: " ++ printOpt(getClipboard()));
    getClipboard() == None;
  });

  setClipboard(None);
  /* Validate yank w/ a register doesn't set clipboard */
  dispatch(KeyboardInput({isText: true, input: "\"ayy"}));
  runEffects();

  wait(~name="Yank 4 w/ register 'a' is NOT sent to clipboard", _ => {
    print_endline("CLIPBOARD: " ++ printOpt(getClipboard()));
    getClipboard() == None;
  });
  setClipboard(None);
  /* Validate yank w/ + register sets clipboard */
  dispatch(KeyboardInput({isText: true, input: "\"+yy"}));
  runEffects();

  wait(~name="Yank 4 w/ register '+' is still sent to clipboard", _ => {
    print_endline("CLIPBOARD: " ++ printOpt(getClipboard()));
    optEqual(getClipboard(), "abc\n");
  });
});
