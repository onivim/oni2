/*
 * HoverStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for the Hover UX
 */

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
      Log.info("Checking for hover...");

      let promise =
        Ext.LanguageFeatures.getDefinition(buffer, position, languageFeatures);

      let _: Lwt.t(unit) =
        Lwt.bind(
          promise,
          result => {
            Log.info(
              "Got definition:"
              ++ Ext.LanguageFeatures.DefinitionResult.toString(result),
            );
            dispatch(Actions.DefinitionAvailable(Core.Buffer.getId(buffer), position, result));
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
      | None => default
      | Some(buf) =>
        let bufferId = Core.Buffer.getId(buf);
        let delay =
          Core.Configuration.getValue(
            c => c.editorHoverDelay,
            state.configuration,
          );

        let position =
          switch (cursors) {
          | [hd, ..._] =>
            let line = Core.Index.ofInt1(hd.line);
            let character = Core.Index.ofInt0(hd.column);
            Core.Position.create(line, character);
          | [] => Core.Position.ofInt0(0, 0)
          };
        let newState = {
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
        (
          newState,
          checkForDefinitionEffect(state.languageFeatures, buf, position),
        );
      }
    | _ => default
    };
  };
  (updater, stream);
};
