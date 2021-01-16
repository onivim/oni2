open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="SCMGitTest", ({wait, _}) => {
  wait(~name="Capture initial state", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  // For now... just validate that the extension activated
  ExtensionHelpers.waitForExtensionToActivate(
    ~extensionId="vscode.git",
    wait,
  );
});
