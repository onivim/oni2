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

// Validate that change the font size via config results in new metrics
runTest(~name="FontSizeChangeTest", (dispatch, wait, runEffects) => {
  wait(~name="Mode switches back to normal", (state: State.t) =>
    state.mode == Vim.Types.Normal
  );
  
  wait(
    ~name="Set configuration to small font size", (state: State.t) => {
    let configuration = state.configuration;
    dispatch(
      ConfigurationSet({
        ...configuration,
        default: {
          ...configuration.default,
          editorFontSize: 8,
        },
      }),
    );
    runEffects();
    true;
  });

  wait(~name="Font is updated", (state: State.t) =>
    state.editorFont.measuredWidth == 5.0
    && state.editorFont.measuredHeight == 10.
    && state.editorFont.fontSize == 8
  );

  wait(
    ~name="Set configuration to large font size", (state: State.t) => {
    let configuration = state.configuration;
    dispatch(
      ConfigurationSet({
        ...configuration,
        default: {
          ...configuration.default,
          editorFontSize: 24,
        },
      }),
    );
    runEffects();
    true;
  });
  
  wait(~name="Font is updated again", (state: State.t) =>
    state.editorFont.measuredWidth == 14.0
    && state.editorFont.measuredHeight == 29.
    && state.editorFont.fontSize == 24
  );
});
