open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

runTest(
  ~name=
    "CopyActiveFilepathToClipboard writes the active buffer's file path to the clipboard",
  (dispatch, wait, runEffects) => {
    setClipboard(Some("def"));
    dispatch(CopyActiveFilepathToClipboard);
    runEffects();

    wait(~name="clipboard contains file path", (state: State.t) => {
      switch (Selectors.getActiveBuffer(state)) {
      | Some(buffer) =>
        switch (getClipboard(), Buffer.getFilePath(buffer)) {
        | (Some(clipboardText), Some(filename)) =>
          String.equal(clipboardText, filename)

        | _ => failwith("no clipboard or file path")
        }
      | _ => failwith("no buffer")
      }
    });
  },
);
