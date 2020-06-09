open Oni_Core;

module Log = (val Log.withNamespace("Service_Exthost"));

// EFFECTS

module Effects = {
  module SCM = {
    let provideOriginalResource = (~handles, extHostClient, path, toMsg) =>
      Isolinear.Effect.createWithDispatch(~name="scm.getOriginalUri", dispatch => {
        // Try our luck with every provider. If several returns Last-Writer-Wins
        // TODO: Is there a better heuristic? Perhaps use rootUri to choose the "nearest" provider?
        handles
        |> List.iter(handle => {
             let promise =
               Exthost.Request.SCM.provideOriginalResource(
                 ~handle,
                 ~uri=Uri.fromPath(path),
                 extHostClient,
               );

             Lwt.on_success(
               promise,
               Option.iter(uri => dispatch(toMsg(uri))),
             );
           })
      });

    let onInputBoxValueChange = (~handle, ~value, extHostClient) =>
      Isolinear.Effect.create(~name="scm.onInputBoxValueChange", () =>
        Exthost.Request.SCM.onInputBoxValueChange(
          ~handle,
          ~value,
          extHostClient,
        )
      );
  };

  module LanguageFeatures = {
    let provideDocumentFormattingEdits =
        (~handle, ~uri, ~options, client, toMsg) => {
      Isolinear.Effect.createWithDispatch(
        ~name="language.provideFormattingEdits", dispatch => {
        let promise =
          Exthost.Request.LanguageFeatures.provideDocumentFormattingEdits(
            ~handle,
            ~resource=uri,
            ~options,
            client,
          );

        Lwt.on_success(
          promise,
          Option.iter(edits => dispatch(toMsg(Ok(edits)))),
        );

        Lwt.on_failure(promise, err =>
          dispatch(toMsg(Error(Printexc.to_string(err))))
        );
      });
    };
  };
};

module MutableState = {
  let activatedFileTypes: Hashtbl.t(string, bool) = Hashtbl.create(16);
};

module Internal = {
  let bufferMetadataToModelAddedDelta = buffer => {
    let lines = Buffer.getLines(buffer) |> Array.to_list;
    let version = Buffer.getVersion(buffer);
    let maybeFilePath = Buffer.getFilePath(buffer);
    let modeId =
      Buffer.getFileType(buffer) |> Option.value(~default="plaintext");

    // The extension host does not like a completely empty buffer,
    // so at least send a single line with an empty string.
    let lines =
      if (lines == []) {
        [""];
      } else {
        lines;
      };

    maybeFilePath
    |> Option.map(filePath => {
         Log.tracef(m => m("Creating model for filetype: %s", modeId));
         Exthost.ModelAddedDelta.create(
           ~versionId=version,
           ~lines,
           ~modeId,
           ~isDirty=true,
           Uri.fromPath(filePath),
         );
       });
  };

  let activateFileType = (~client, maybeFileType: option(string)) =>
    maybeFileType
    |> Option.iter(ft =>
         if (!Hashtbl.mem(MutableState.activatedFileTypes, ft)) {
           // If no entry, we haven't activated yet
           Exthost.Request.ExtensionService.activateByEvent(
             ~event="onLanguage:" ++ ft,
             client,
           );
           Hashtbl.add(MutableState.activatedFileTypes, ft, true);
         }
       );
};

// SUBSCRIPTIONS

module Sub = {
  type bufferParams = {
    client: Exthost.Client.t,
    buffer: Oni_Core.Buffer.t,
  };

  module BufferSubscription =
    Isolinear.Sub.Make({
      type nonrec msg = unit;
      type nonrec params = bufferParams;
      type state = {didAdd: bool};

      let name = "Service_Exthost.BufferSubscription";
      let id = params => {
        params.buffer |> Oni_Core.Buffer.getId |> string_of_int;
      };

      let init = (~params, ~dispatch as _) => {
        let bufferId = Oni_Core.Buffer.getId(params.buffer);

        Log.infof(m => m("Starting buffer subscription for: %d", bufferId));

        params.buffer
        |> Oni_Core.Buffer.getFileType
        |> Internal.activateFileType(~client=params.client);

        let maybeMetadata =
          Internal.bufferMetadataToModelAddedDelta(params.buffer);

        switch (maybeMetadata) {
        | Some(metadata) =>
          let addedDelta =
            Exthost.DocumentsAndEditorsDelta.create(
              ~removedDocuments=[],
              ~addedDocuments=[metadata],
              (),
            );

          Exthost.Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
            ~delta=addedDelta,
            params.client,
          );
          {didAdd: true};
        | None => {didAdd: false}
        };
      };

      let update = (~params as _, ~state, ~dispatch as _) => {
        state;
      };

      let dispose = (~params, ~state) =>
        if (state.didAdd) {
          params.buffer
          |> Oni_Core.Buffer.getFilePath
          |> Option.iter(filePath => {
               let removedDelta =
                 Exthost.DocumentsAndEditorsDelta.create(
                   ~removedDocuments=[Uri.fromPath(filePath)],
                   ~addedDocuments=[],
                   (),
                 );
               Exthost.Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
                 ~delta=removedDelta,
                 params.client,
               );
             });
        };
    });

  let buffer = (~buffer, ~client) =>
    BufferSubscription.create({buffer, client});

  type editorParams = {
    client: Exthost.Client.t,
    editor: Exthost.TextEditor.AddData.t,
  };
  module EditorSubscription =
    Isolinear.Sub.Make({
      type nonrec msg = unit;
      type nonrec params = editorParams;

      type state = {id: string};

      let name = "Service_Exthost.EditorSubscription";
      let id = params => {
        params.editor.id;
      };

      let init = (~params, ~dispatch as _) => {
        let addedDelta =
          Exthost.DocumentsAndEditorsDelta.create(
            ~addedEditors=[params.editor],
            (),
          );

        Log.infof(m =>
          m("Starting editor subscription for: %s", params.editor.id)
        );
        Exthost.Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
          ~delta=addedDelta,
          params.client,
        );
        {id: params.editor.id};
      };

      let update = (~params as _, ~state, ~dispatch as _) => {
        state;
      };

      let dispose = (~params, ~state) => {
        Log.infof(m =>
          m("Stopping editor subscription for: %s", params.editor.id)
        );
        let removedDelta =
          Exthost.DocumentsAndEditorsDelta.create(
            ~removedEditors=[state.id],
            (),
          );
        Exthost.Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
          ~delta=removedDelta,
          params.client,
        );
      };
    });

  let editor = (~editor, ~client) =>
    EditorSubscription.create({editor, client});

  type activeEditorParams = {
    client: Exthost.Client.t,
    activeEditorId: string,
  };

  module ActiveEditorSubscription =
    Isolinear.Sub.Make({
      type nonrec msg = unit;
      type nonrec params = activeEditorParams;

      type state = {lastId: string};

      let name = "Service_Exthost.ActiveEditorSubscription";
      let id = _ => "ActiveEditorSubscription";

      let setActiveEditor = (~activeEditorId, ~client) => {
        Log.infof(m => m("Setting active editor id: %s", activeEditorId));
        let activeEditor =
          Exthost.DocumentsAndEditorsDelta.create(
            ~newActiveEditor=Some(activeEditorId),
            (),
          );

        Exthost.Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
          ~delta=activeEditor,
          client,
        );
      };

      let init = (~params, ~dispatch as _) => {
        setActiveEditor(
          ~activeEditorId=params.activeEditorId,
          ~client=params.client,
        );

        {lastId: params.activeEditorId};
      };

      let update = (~params, ~state, ~dispatch as _) =>
        if (params.activeEditorId != state.lastId) {
          setActiveEditor(
            ~activeEditorId=params.activeEditorId,
            ~client=params.client,
          );
          {lastId: params.activeEditorId};
        } else {
          state;
        };

      let dispose = (~params as _, ~state as _) => {
        ();
      };
    });

  let activeEditor = (~activeEditorId, ~client) => {
    ActiveEditorSubscription.create({activeEditorId, client});
  };
};
