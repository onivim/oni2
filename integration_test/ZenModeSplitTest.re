open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="ZenModeSplitTest", ({dispatch, wait, _}) => {
  dispatch(Actions.Zen(Feature_Zen.Testing.enableZenMode));

  wait(~name="Wait until ZenMode is enabled", (state: State.t) =>
    Feature_Zen.isZen(state.zen)
  );

  runCommand(
    ~dispatch,
    Feature_Layout.Commands.splitVertical
    |> Core.Command.map(msg => Model.Actions.Layout(msg)),
  );

  wait(~name="Wait until Split is created", (state: State.t) =>
    !Feature_Zen.isZen(state.zen)
  );
});
