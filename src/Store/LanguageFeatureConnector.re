/*
 * LanguageFeatureConnector.re
 *
 * This implements an updater (reducer + side effects) for language features
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Model;
open Actions;

module Utility = Utility;
module Ext = Oni_Extensions;

module DefinitionResult = LanguageFeatures.DefinitionResult;
module Option = Utility.Option;

module Log = (val Log.withNamespace("Oni2.Store.LanguageFeatures"));

let start = () => {
  let (stream, _dispatch) = Isolinear.Stream.create();

  let checkForDefinitionEffect = (languageFeatures, buffer, location) =>
    Isolinear.Effect.createWithDispatch(
      ~name="languageFeature.checkForDefinition", dispatch => {
      Log.debug("Checking for definition...");

      let getDefinitionPromise =
        LanguageFeatures.requestDefinition(
          ~buffer,
          ~location,
          languageFeatures,
        );

      let getHighlightsPromise =
        LanguageFeatures.requestDocumentHighlights(
          ~buffer,
          ~location,
          languageFeatures,
        );

      let id = Buffer.getId(buffer);

      Lwt.on_success(getHighlightsPromise, result => {
        dispatch(
          BufferHighlights(
            BufferHighlights.DocumentHighlightsAvailable(id, result),
          ),
        )
      });

      Lwt.on_failure(getHighlightsPromise, _exn => {
        dispatch(
          BufferHighlights(BufferHighlights.DocumentHighlightsCleared(id)),
        )
      });

      Lwt.on_success(getDefinitionPromise, result =>
        dispatch(DefinitionAvailable(id, location, result))
      );
    });

  let findAllReferences = state =>
    Isolinear.Effect.createWithDispatch(
      ~name="languageFeature.findAllReferences", dispatch => {
      let maybeBuffer = state |> Selectors.getActiveBuffer;

      let maybeEditor =
        state |> Selectors.getActiveEditorGroup |> Selectors.getActiveEditor;

      Option.iter2(
        (buffer, editor) => {
          let location = Editor.getPrimaryCursor(editor);
          let promise =
            LanguageFeatures.requestFindAllReferences(
              ~buffer,
              ~location,
              state.languageFeatures,
            );

          Lwt.on_success(promise, result => {
            dispatch(References(References.Set(result)))
          });
        },
        maybeBuffer,
        maybeEditor,
      );
    });

  let updater = (state: State.t, action: Actions.t) => {
    let default = (state, Isolinear.Effect.none);
    switch (action) {
    | Tick({deltaTime, _}) =>
      if (Hover.isAnimationActive(state.hover)) {
        let hover = state.hover |> Hover.tick(deltaTime);
        let newState = {...state, hover};

        (newState, Isolinear.Effect.none);
      } else {
        default;
      }

    | References(References.Requested) => (state, findAllReferences(state))
    | References(References.Set(references)) => (
        {...state, references},
        Isolinear.Effect.none,
      )
    | EditorCursorMove(_, cursors) when state.mode != Vim.Types.Insert =>
      switch (Selectors.getActiveBuffer(state)) {
      | None => (state, Isolinear.Effect.none)
      | Some(buf) =>
        let bufferId = Buffer.getId(buf);
        let delay =
          Configuration.getValue(
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
            Hover.show(
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
