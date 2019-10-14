/*
 * MerlinStoreConnector.re
 *
 * This module connects merlin to the Store,
 * for some initial, basic language integration
 */

module Core = Oni_Core;
module Extensions = Oni_Extensions;
module Model = Oni_Model;

module Log = Core.Log;
module Zed_utf8 = Core.ZedBundled;

open Oni_Merlin;

let runOnMainThread = (cb, arg) => {
  Revery.App.runOnMainThread(cb(arg));
};

let effectIfMerlinEnabled = effect => {
  let ret = (configuration: Core.Configuration.t) => {
    let isMerlinEnabled =
      Core.Configuration.getValue(c => c.experimentalMerlin, configuration);

    if (isMerlinEnabled) {effect} else {Isolinear.Effect.none};
  };

  ret;
};

let start = () => {
  let (stream, dispatch) = Isolinear.Stream.create();
  let modelChangedEffect =
      (buffers: Model.Buffers.t, bu: Core.Types.BufferUpdate.t) =>
    effectIfMerlinEnabled(
      Isolinear.Effect.create(~name="exthost.bufferUpdate", () =>
        switch (Model.Buffers.getBuffer(bu.id, buffers)) {
        | None => ()
        | Some(v) =>
          let lines = Model.Buffer.getLines(v);
          let fileType = Model.Buffer.getFileType(v);
          switch (fileType, Model.Buffer.getFilePath(v)) {
          | (Some(ft), Some(path)) when ft == "reason" || ft == "ocaml" =>
            let cb = err => {
              let modelDiagnostics =
                MerlinProtocolConverter.toModelDiagnostics(err);
              Log.info(
                "Got "
                ++ string_of_int(List.length(modelDiagnostics))
                ++ " errors",
              );
              Revery.App.runOnMainThread(() =>
                dispatch(
                  Model.Actions.DiagnosticsSet(v, "merlin", modelDiagnostics),
                )
              );
            };
            let _ =
              MerlinRequestQueue.getErrors(Sys.getcwd(), path, lines, cb);
            ();
          | _ => ()
          };
        }
      ),
    );

  let updater = (state: Model.State.t, action) => {
    switch (action) {
    | Model.Actions.BufferUpdate(bu) => (
        state,
        modelChangedEffect(state.buffers, bu, state.configuration),
      )
    | _ => (state, Isolinear.Effect.none)
    };
  };

  (updater, stream);
};
