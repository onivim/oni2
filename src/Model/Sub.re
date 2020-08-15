open Oni_Core.Utility;

module type Config = {type computedValue;};

type params('a) = {
  id: string,
  selector: State.t => 'a,
  state: State.t,
  toMsg: 'a => Actions.t,
};

module Make = (ConfigInstance: Config) =>
  Isolinear.Sub.Make({
    type nonrec msg = Actions.t;

    type nonrec params = params(ConfigInstance.computedValue);

    type state = {lastValue: ConfigInstance.computedValue};

    let name = "Oni_Model.StateDeltaSub";
    let id = ({id, _}) => id;

    let init = (~params, ~dispatch) => {
      let lastValue = params.selector(params.state);
      dispatch(params.toMsg(lastValue));
      {lastValue: lastValue};
    };

    let update = (~params, ~state, ~dispatch) => {
      let newValue = params.selector(params.state);
      if (newValue != state.lastValue) {
        dispatch(params.toMsg(newValue));
      };

      {lastValue: newValue};
    };

    let dispose = (~params as _, ~state as _) => {
      ();
    };
  });

module ActiveFilePathSub =
  Make({
    type computedValue = option(string);
  });

let activeFile = (~id, ~state: State.t, ~toMsg) => {
  let selector = state =>
    state
    |> Selectors.getActiveBuffer
    |> OptionEx.flatMap(Oni_Core.Buffer.getFilePath);
  ActiveFilePathSub.create(
    {state, selector, id, toMsg}: ActiveFilePathSub.params,
  );
};
