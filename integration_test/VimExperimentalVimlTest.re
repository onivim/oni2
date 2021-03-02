open Oni_Model;
open Oni_IntegrationTestLib;

let configuration = {|
{
    "experimental.viml": ["set smartcase", "set ignorecase", "echo 'hello-world'"]
}
|};

runTest(~configuration=Some(configuration), ~name="VimExperimentalVimlTest", ({dispatch, wait, runEffects, input, _}) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  wait(~name="notification shows up", (state: State.t) => {
    let notifications = Feature_Notification.all(state.notifications);

    notifications
    |> List.exists(notification => Feature_Notification.(notification.message == "hello-world"));
  });

});
