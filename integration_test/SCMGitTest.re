open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="SCMGitTest", (_dispatch, wait, _runEffects) => {
  wait(~name="Capture initial state", (state: State.t) =>
    state.vimMode == Vim.Types.Normal
  );

  // For now... just validate that the extension activated
  ExtensionHelpers.waitForExtensionToActivate(
    ~extensionId="vscode.git",
    wait,
  );
});
