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
module Protocol = Extensions.ExtensionHostProtocol;

let start = (extensions, setup: Core.Setup.t) => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let onExtHostClosed = () => print_endline("ext host closed");

  let extensionInfo =
    extensions
    |> List.map(ext =>
         Extensions.ExtensionHostInitData.ExtensionInfo.ofScannedExtension(
           ext,
         )
       );

  let onMessage = (scope, method, args) => {
    switch (scope, method, args) {
    | (
        "MainThreadStatusBar",
        "$setEntry",
        [
          `Int(id),
          _,
          `String(text),
          _,
          _,
          _,
          `Int(alignment),
          `Int(priority),
        ],
      ) =>
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
      Ok(None);
    | _ => Ok(None)
    };
  };

  let initData = ExtensionHostInitData.create(~extensions=extensionInfo, ());
  let extHostClient =
    Extensions.ExtensionHostClient.start(
      ~initData,
      ~onClosed=onExtHostClosed,
      ~onMessage,
      setup,
    );

  let _bufferMetadataToModelAddedDelta = (bm: Core.Types.BufferMetadata.t) =>
    switch (bm.filePath, bm.fileType) {
    | (Some(fp), Some(ft)) =>
      Some(
        Protocol.ModelAddedDelta.create(
          ~uri=Protocol.Uri.createFromFilePath(fp),
          ~versionId=bm.version,
          ~lines=[],
          ~modeId=ft,
          ~isDirty=bm.modified,
          (),
        ),
      )
    /* TODO: filetype detection */
    | (Some(fp), _) =>
      Some(
        Protocol.ModelAddedDelta.create(
          ~uri=Protocol.Uri.createFromFilePath(fp),
          ~versionId=bm.version,
          ~lines=[],
          ~modeId="unknown",
          ~isDirty=bm.modified,
          (),
        ),
      )
    | _ => None
    };

  let pumpEffect =
    Isolinear.Effect.create(~name="exthost.pump", () =>
      ExtensionHostClient.pump(extHostClient)
    );

  let sendBufferEnterEffect = (bu: Core.Types.BufferNotification.t) =>
    Isolinear.Effect.create(~name="exthost.bufferEnter", () => {
      let metadata =
        Core.Types.BufferNotification.getBufferMetadataOpt(bu.bufferId, bu);
      switch (metadata) {
      | None => ()
      | Some(bm) =>
        switch (_bufferMetadataToModelAddedDelta(bm)) {
        | None => ()
        | Some(v) =>
          ExtensionHostClient.send(
            extHostClient,
            Protocol.OutgoingNotifications.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
              ~removedDocuments=[],
              ~addedDocuments=[v],
              (),
            ),
          )
        }
      };
    });

  let updater = (state: Model.State.t, action) =>
    switch (action) {
    | Model.Actions.BufferEnter(bm) => (state, sendBufferEnterEffect(bm))
    | Model.Actions.Tick => (state, pumpEffect)
    | _ => (state, Isolinear.Effect.none)
    };

  (updater, stream);
};
