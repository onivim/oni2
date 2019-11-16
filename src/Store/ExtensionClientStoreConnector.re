/*
 * ExtensionClientStoreConnector.re
 *
 * This connects the extension client to the store:
 * - Converts extension host notifications into ACTIONS
 * - Calls appropriate APIs on extension host based on ACTIONS
 */

module Core = Oni_Core;
module Model = Oni_Model;

open Oni_Extensions;
module Extensions = Oni_Extensions;
module Protocol = Extensions.ExtHostProtocol;

let start = (extensions, setup: Core.Setup.t) => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let onExtHostClosed = () => Core.Log.info("ext host closed");

  let extensionInfo =
    extensions
    |> List.map(ext =>
         Extensions.ExtHostInitData.ExtensionInfo.ofScannedExtension(ext)
       );

  let onDiagnosticsClear = owner => {
    dispatch(Model.Actions.DiagnosticsClear(owner));
  };

  let onDiagnosticsChangeMany =
      (diagCollection: Protocol.DiagnosticsCollection.t) => {
    let protocolDiagToDiag: Protocol.Diagnostic.t => Model.Diagnostic.t =
      d => {
        let range = Protocol.OneBasedRange.toRange(d.range);
        let message = d.message;
        Model.Diagnostic.create(~range, ~message, ());
      };

    let f = (d: Protocol.Diagnostics.t) => {
      let diagnostics = List.map(protocolDiagToDiag, snd(d));
      let uri = fst(d);
      Model.Actions.DiagnosticsSet(uri, diagCollection.name, diagnostics);
    };

    diagCollection.perFileDiagnostics
    |> List.map(f)
    |> List.iter(a => dispatch(a));
  };

  let onStatusBarSetEntry = ((id, text, alignment, priority)) => {
    dispatch(
      Model.Actions.StatusBarAddItem(
        Model.StatusBarModel.Item.create(
          ~id,
          ~text,
          ~alignment=Model.StatusBarModel.Alignment.ofInt(alignment),
          ~priority,
          (),
        ),
      ),
    );
  };

  let onOutput = msg => {
    Core.Log.info("[ExtHost]: " ++ msg);
  };

  let initData = ExtHostInitData.create(~extensions=extensionInfo, ());
  let extHostClient =
    Extensions.ExtHostClient.start(
      ~initData,
      ~onClosed=onExtHostClosed,
      ~onStatusBarSetEntry,
      ~onDiagnosticsClear,
      ~onDiagnosticsChangeMany,
      ~onOutput,
      setup,
    );

  let _bufferMetadataToModelAddedDelta =
      (bm: Vim.BufferMetadata.t, fileType: option(string)) =>
    switch (bm.filePath, fileType) {
    | (Some(fp), Some(ft)) =>
      Core.Log.info("Creating model for filetype: " ++ ft);
      Some(
        Protocol.ModelAddedDelta.create(
          ~uri=Core.Uri.fromPath(fp),
          ~versionId=bm.version,
          ~lines=[""],
          ~modeId=ft,
          ~isDirty=true,
          (),
        ),
      );
    | _ => None
    };

  let pumpEffect =
    Isolinear.Effect.create(~name="exthost.pump", () =>
      ExtHostClient.pump(extHostClient)
    );

  let sendBufferEnterEffect =
      (bm: Vim.BufferMetadata.t, fileType: option(string)) =>
    Isolinear.Effect.create(~name="exthost.bufferEnter", () =>
      switch (_bufferMetadataToModelAddedDelta(bm, fileType)) {
      | None => ()
      | Some(v) => ExtHostClient.addDocument(v, extHostClient)
      }
    );

  let modelChangedEffect =
      (buffers: Model.Buffers.t, bu: Core.Types.BufferUpdate.t) =>
    Isolinear.Effect.create(~name="exthost.bufferUpdate", () =>
      switch (Model.Buffers.getBuffer(bu.id, buffers)) {
      | None => ()
      | Some(v) =>
        Core.Log.perf("exthost.bufferUpdate", () => {
          let modelContentChange =
            Protocol.ModelContentChange.ofBufferUpdate(
              bu,
              Protocol.Eol.default,
            );
          let modelChangedEvent =
            Protocol.ModelChangedEvent.create(
              ~changes=[modelContentChange],
              ~eol=Protocol.Eol.default,
              ~versionId=bu.version,
              (),
            );

          let uri = Model.Buffer.getUri(v);

          ExtHostClient.updateDocument(
            uri,
            modelChangedEvent,
            true,
            extHostClient,
          );
        })
      }
    );

  let registerQuitCleanupEffect =
    Isolinear.Effect.createWithDispatch(
      ~name="exthost.registerQuitCleanup", dispatch =>
      dispatch(
        Model.Actions.RegisterQuitCleanup(
          () => ExtHostClient.close(extHostClient),
        ),
      )
    );

  let updater = (state: Model.State.t, action) =>
    switch (action) {
    | Model.Actions.Init => (state, registerQuitCleanupEffect)
    | Model.Actions.BufferUpdate(bu) => (
        state,
        modelChangedEffect(state.buffers, bu),
      )
    | Model.Actions.BufferEnter(bm, fileTypeOpt) => (
        state,
        sendBufferEnterEffect(bm, fileTypeOpt),
      )
    | Model.Actions.Tick(_) => (state, pumpEffect)
    | _ => (state, Isolinear.Effect.none)
    };

  (updater, stream);
};
