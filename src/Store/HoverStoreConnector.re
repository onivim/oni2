/*
 * HoverStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for the Hover UX
 */

open EditorCoreTypes;
module Core = Oni_Core;
module Ext = Oni_Extensions;
module Model = Oni_Model;

module Actions = Model.Actions;
module Animation = Model.Animation;
module Quickmenu = Model.Quickmenu;

module Log = (val Oni_Core.Log.withNamespace("Oni2.HoverStoreConnector"));

let start = () => {
  let (stream, _dispatch) = Isolinear.Stream.create();

  let checkForDefinitionEffect = (languageFeatures, buffer, position) =>
    Isolinear.Effect.createWithDispatch(
      ~name="hover.checkForDefinition", dispatch => {
      Log.info("Checking for definition...");

      let promise =
        Model.LanguageFeatures.requestDefinition(
          ~buffer,
          ~location=position,
          languageFeatures,
        );

      let _: Lwt.t(unit) =
        Lwt.bind(
          promise,
          result => {
            Log.info(
              "Got definition:"
              ++ Model.LanguageFeatures.DefinitionResult.toString(result),
            );
            dispatch(
              Actions.DefinitionAvailable(
                Core.Buffer.getId(buffer),
                position,
                result,
              ),
            );
            Lwt.return();
          },
        );
      ();
    });

  let updater = (state: Model.State.t, action: Actions.t) => {
    let default = (state, Isolinear.Effect.none);
    switch (action) {
    | Actions.Tick({deltaTime, _}) =>
      if (Model.Hover.isAnimationActive(state.hover)) {
        let hover = state.hover |> Model.Hover.tick(deltaTime);
        let newState = {...state, hover};

        (newState, Isolinear.Effect.none);
      } else {
        default;
      }
    | Actions.EditorCursorMove(_, cursors) when state.mode != Vim.Types.Insert =>
      switch (Model.Selectors.getActiveBuffer(state)) {
      | None => (state, Isolinear.Effect.none)
      | Some(buf) =>
        let bufferId = Core.Buffer.getId(buf);
        let delay =
          Core.Configuration.getValue(
            c => c.editorHoverDelay,
            state.configuration,
          );

        let location =
          switch (cursors) {
          | [cursor, ..._] => (cursor :> Location.t)
          | [] => Location.{line: Index.zero, column: Index.zero}
          };
        let newState = {
          ...state,
          hover:
            Model.Hover.show(
              ~bufferId,
              ~location,
              ~currentTime=Unix.gettimeofday(),
              ~delay=float_of_int(delay) /. 1000.,
              (),
            ),
        };
        (
          newState,
          checkForDefinitionEffect(state.languageFeatures, buf, location),
        );
      }
    | _ => default
    };
  };
  (updater, stream);
};
