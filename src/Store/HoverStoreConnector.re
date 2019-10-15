/*
 * HoverStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for the Hover UX
 */

module Core = Oni_Core;
module Model = Oni_Model;

module Actions = Model.Actions;
module Animation = Model.Animation;
module Menu = Model.Menu;
module MenuJob = Model.MenuJob;


let start = () => {
  let (stream, _dispatch) = Isolinear.Stream.create();

  let updater = (state: Model.State.t, action: Actions.t) => {
    
    switch (action) {
    | Actions.Tick({deltaTime, _}) =>
      let hover = state.hover |> Model.Hover.tick(deltaTime)
      let newState = {
        ...state,
        hover,
      };
      
      (newState, Isolinear.Effect.none);
    | Actions.CursorMove(position) =>
      let newState = switch (Model.Selectors.getActiveBuffer(state)) {
      | None => state
      | Some(buf) =>
        let bufferId = Model.Buffer.getId(buf);
        {
          ...state,
          hover: Model.Hover.show(~bufferId, ~position, ~currentTime=Unix.gettimeofday(), ()),
        };
      };
      (newState, Isolinear.Effect.none);
    | _ => (state, Isolinear.Effect.none);
    };

  };
  (updater, stream);
};
