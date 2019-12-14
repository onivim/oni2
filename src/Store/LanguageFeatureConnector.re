/*
 * LanguageFeatureConnector.re
 *
 * This implements an updater (reducer + side effects) for language features
 */

open EditorCoreTypes;
module Core = Oni_Core;
module Utility = Core.Utility;
module Ext = Oni_Extensions;
module Model = Oni_Model;

module Actions = Model.Actions;
module Animation = Model.Animation;
module BufferHighlights = Model.BufferHighlights;
module Quickmenu = Model.Quickmenu;

module Log = (
  val Oni_Core.Log.withNamespace("Oni2.LanguageFeatureConnector")
);

let start = () => {
  let (stream, _dispatch) = Isolinear.Stream.create();

  let checkForDefinitionEffect = (languageFeatures, buffer, location) =>
    Isolinear.Effect.createWithDispatch(
      ~name="languageFeature.checkForDefinition", dispatch => {
      Log.info("Checking for definition...");

      let getDefinitionPromise =
        Model.LanguageFeatures.requestDefinition(
          ~buffer,
          ~location,
          languageFeatures,
        );

      let getHighlightsPromise =
        Model.LanguageFeatures.requestDocumentHighlights(
          ~buffer,
          ~location,
          languageFeatures,
        );

      let id = Core.Buffer.getId(buffer);
      let () =
        Lwt.on_success(getHighlightsPromise, result => {
          dispatch(
            Actions.BufferHighlights(
              BufferHighlights.DocumentHighlightsAvailable(id, result),
            ),
          )
        });
      let () =
        Lwt.on_failure(getHighlightsPromise, _exn => {
          dispatch(
            Actions.BufferHighlights(
              BufferHighlights.DocumentHighlightsCleared(id),
            ),
          )
        });

      let () =
        Lwt.on_success(
          getDefinitionPromise,
          result => {
            Log.info(
              "Got definition:"
              ++ Model.LanguageFeatures.DefinitionResult.toString(result),
            );
            dispatch(Actions.DefinitionAvailable(id, location, result));
          },
        );
      ();
    });

  let findAllReferences = state =>
    Isolinear.Effect.createWithDispatch(
      ~name="languageFeature.findAllReferences", dispatch => {
      let maybeBuffer = state |> Model.Selectors.getActiveBuffer;

      let maybeEditor =
        state
        |> Model.Selectors.getActiveEditorGroup
        |> Model.Selectors.getActiveEditor;

      let request = (buffer, editor) => {
        let location = Model.Editor.getPrimaryCursor(editor);
        let promise =
          Model.LanguageFeatures.requestFindAllReferences(
            ~buffer,
            ~location,
            state.languageFeatures,
          );

        Lwt.on_success(promise, result => {
          dispatch(Actions.FindAllReferencesSet(result))
        });
      };

      Utility.Option.iter2(request, maybeBuffer, maybeEditor);
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
    | Actions.FindAllReferencesRequested => (state, findAllReferences(state))
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
