/*
 * HoverStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for the Hover UX
 */

open EditorCoreTypes;
module Core = Oni_Core;
module Model = Oni_Model;

module Actions = Model.Actions;
module Animation = Model.Animation;
module Quickmenu = Model.Quickmenu;

let start = () => {
  let (stream, _dispatch) = Isolinear.Stream.create();

  let updater = (state: Model.State.t, action: Actions.t) => {
    switch (action) {
    | Actions.Tick({deltaTime, _}) =>
      if (Model.Hover.isAnimationActive(state.hover)) {
        let hover = state.hover |> Model.Hover.tick(deltaTime);
        let newState = {...state, hover};

        (newState, Isolinear.Effect.none);
      } else {
        (state, Isolinear.Effect.none);
      }
    | Actions.EditorCursorMove(_, cursors) when state.mode != Vim.Types.Insert =>
      let newState =
        switch (Model.Selectors.getActiveBuffer(state)) {
        | None => state
        | Some(buf) =>
          let bufferId = Core.Buffer.getId(buf);
          let delay =
            Core.Configuration.getValue(
              c => c.editorHoverDelay,
              state.configuration,
            );

          let position =
            switch (cursors) {
            | [cursor, ..._] => (cursor :> Location.t)
            | [] => Location.{line: Index.zero, column: Index.zero}
            };
          {
            ...state,
            hover:
              Model.Hover.show(
                ~bufferId,
                ~position,
                ~currentTime=Unix.gettimeofday(),
                ~delay=float_of_int(delay) /. 1000.,
                (),
              ),
          };
        };
      (newState, Isolinear.Effect.none);
    | _ => (state, Isolinear.Effect.none)
    };
  };
  (updater, stream);
};
