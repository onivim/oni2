open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

// Validate that change the font size via config results in new metrics
runTest(~name="FontSizeChangeTest", (dispatch, wait, runEffects) => {
  wait(~name="Mode switches back to normal", (state: State.t) =>
    state.mode == Vim.Types.Normal
  );

  wait(~name="Set configuration to small font size", (state: State.t) => {
    let configuration = state.configuration;
    dispatch(
      ConfigurationSet({
        ...configuration,
        default: {
          ...configuration.default,
          editorFontSize: 8.,
        },
      }),
    );
    runEffects();
    true;
  });

  let isClose = (f1, f2) => Float.abs(f1 -. f2) < 0.000001;

  wait(~name="Font is updated", ({editorFont, _}: State.t) => {
    print_endline(
      "Font metrics, round 1: " ++ EditorFont.toString(editorFont),
    );

    isClose(editorFont.measuredWidth, 4.8)
    && isClose(editorFont.measuredHeight, 9.599976)
    && isClose(editorFont.fontSize, 8.0);
  });

  wait(~name="Set configuration to large font size", (state: State.t) => {
    let configuration = state.configuration;
    dispatch(
      ConfigurationSet({
        ...configuration,
        default: {
          ...configuration.default,
          editorFontSize: 24.,
        },
      }),
    );
    runEffects();
    true;
  });

  wait(~name="Font is updated again", ({editorFont, _}: State.t) => {
    print_endline(
      "Font metrics, round 2: " ++ EditorFont.toString(editorFont),
    );
    isClose(editorFont.measuredWidth, 14.4)
    && isClose(editorFont.measuredHeight, 28.799927)
    && isClose(editorFont.fontSize, 24.);
  });
});
