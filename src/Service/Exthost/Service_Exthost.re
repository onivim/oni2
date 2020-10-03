open Oni_Core;

module Log = (val Log.withNamespace("Service_Exthost"));

// EFFECTS

module Effects = {
  module Commands = {
    let executeContributedCommand = (~command, ~arguments, client) =>
      Isolinear.Effect.create(
        ~name="exthost.commands.executeContributedCommand", () => {
        Exthost.Request.Commands.executeContributedCommand(
          ~command,
          ~arguments,
          client,
        )
      });
  };
  module Decorations = {
    let provideDecorations = (~handle, ~requests, ~toMsg, client) => {
      Isolinear.Effect.createWithDispatch(
        ~name="exthost.provideDecorations", dispatch => {
        let promise =
          Exthost.Request.Decorations.provideDecorations(
            ~handle,
            ~requests,
            client,
          );

        Lwt.on_success(promise, decorations => dispatch(toMsg(decorations)));
      });
    };
  };
  module Documents = {
    let modelChanged =
        (
          ~previousBuffer,
          ~buffer: Buffer.t,
          ~update: BufferUpdate.t,
          client,
          toMsg,
        ) =>
      Isolinear.Effect.createWithDispatch(
        ~name="exthost.bufferUpdate", dispatch =>
        Oni_Core.Log.perf("exthost.bufferUpdate", () => {
          let modelContentChange =
            Exthost.ModelContentChange.ofBufferUpdate(
              ~previousBuffer,
              update,
              Exthost.Eol.default,
            );
          let modelChangedEvent =
            Exthost.ModelChangedEvent.{
              changes: [modelContentChange],
              eol: Exthost.Eol.default,
              versionId: update.version,
            };

          Exthost.Request.Documents.acceptModelChanged(
            ~uri=Buffer.getUri(buffer),
            ~modelChangedEvent,
            ~isDirty=Buffer.isModified(buffer),
            client,
          );
          dispatch(toMsg());
        })
      );
  };
  module FileSystemEventService = {
    let onFileEvent = (~events, extHostClient) =>
      Isolinear.Effect.create(~name="fileSystemEventService.onFileEvent", () => {
        Exthost.Request.FileSystemEventService.onFileEvent(
          ~events,
          extHostClient,
        )
      });
  };

  module SCM = {
    let getOriginalContent = (~handle, ~uri, ~toMsg, client) =>
      Isolinear.Effect.createWithDispatch(
        ~name="scm.getOriginalSourceLines", dispatch => {
        let promise =
          Exthost.Request.DocumentContentProvider.provideTextDocumentContent(
            ~handle,
            ~uri,
            client,
          );

        Lwt.on_success(promise, maybeContent => {
          switch (maybeContent) {
          | None => ()
          | Some(content) =>
            let lines =
              content |> Str.(split(regexp("\r?\n"))) |> Array.of_list;

            dispatch(toMsg(lines));
          }
        });
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

    let provideDocumentRangeFormattingEdits =
        (~handle, ~uri, ~range, ~options, client, toMsg) => {
      Isolinear.Effect.createWithDispatch(
        ~name="language.provideRangeFormattingEdits", dispatch => {
        let promise =
          Exthost.(
            Request.LanguageFeatures.provideDocumentRangeFormattingEdits(
              ~handle,
              ~resource=uri,
              ~options,
              ~range=range |> OneBasedRange.ofRange,
              client,
            )
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

    let provideHover = (~handle, ~uri, ~position, client, toMsg) => {
      Isolinear.Effect.createWithDispatch(
        ~name="language.provideHover", dispatch => {
        let promise =
          Exthost.Request.LanguageFeatures.provideHover(
            ~handle,
            ~resource=uri,
            ~position=Exthost.OneBasedPosition.ofPosition(position),
            client,
          );

        Lwt.on_success(
          promise,
          Option.iter(hover => dispatch(toMsg(Ok(hover)))),
        );

        Lwt.on_failure(promise, err =>
          dispatch(toMsg(Error(Printexc.to_string(err))))
        );
      });
    };

    let provideReferences =
        (~handle, ~uri, ~position, ~context, client, toMsg) => {
      Isolinear.Effect.createWithDispatch(
        ~name="language.provideReferences", dispatch => {
        let promise =
          Exthost.Request.LanguageFeatures.provideReferences(
            ~handle,
            ~resource=uri,
            ~position=Exthost.OneBasedPosition.ofPosition(position),
            ~context,
            client,
          );

        Lwt.on_success(promise, locations =>
          dispatch(toMsg(Ok(locations)))
        );

        Lwt.on_failure(promise, err =>
          dispatch(toMsg(Error(Printexc.to_string(err))))
        );
      });
    };

    let provideSignatureHelp =
        (~handle, ~uri, ~position, ~context, client, toMsg) => {
      Isolinear.Effect.createWithDispatch(
        ~name="language.provideSignatureHelp", dispatch => {
        let promise =
          Exthost.Request.LanguageFeatures.provideSignatureHelp(
            ~handle,
            ~resource=uri,
            ~position=Exthost.OneBasedPosition.ofPosition(position),
            ~context,
            client,
          );

        Lwt.on_success(promise, sigHelp => dispatch(Ok(sigHelp) |> toMsg));

        Lwt.on_failure(promise, err =>
          dispatch(Error(Printexc.to_string(err)) |> toMsg)
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
    let modeId = Buffer.getFileType(buffer) |> Buffer.FileType.toString;

    // The extension host does not like a completely empty buffer,
    // so at least send a single line with an empty string.
    let lines =
      if (lines == []) {
        [""];
      } else {
        // There needs to be an empty line at the end of the buffer to sync changes at the end
        lines @ [""];
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

  let activateFileType = (~client, fileType: string) =>
    if (!Hashtbl.mem(MutableState.activatedFileTypes, fileType)) {
      // If no entry, we haven't activated yet
      Exthost.Request.ExtensionService.activateByEvent(
        ~event="onLanguage:" ++ fileType,
        client,
      );
      Hashtbl.add(MutableState.activatedFileTypes, fileType, true);
    };
};

// SUBSCRIPTIONS

module Sub = {
  module SCM = {
    type originalUriParams = {
      filePath: string,
      handle: int,
      client: Exthost.Client.t,
    };

    module OriginalUriSubscription =
      Isolinear.Sub.Make({
        type nonrec msg = Uri.t;
        type nonrec params = originalUriParams;
        type state = unit;

        let name = "Service_Exthost.SCM.OriginalUriSubscription";
        let id = params => string_of_int(params.handle) ++ params.filePath;

        let init = (~params, ~dispatch) => {
          let promise =
            Exthost.Request.SCM.provideOriginalResource(
              ~handle=params.handle,
              ~uri=Uri.fromPath(params.filePath),
              params.client,
            );

          Lwt.on_success(
            promise,
            fun
            | None => ()
            | Some(uri) => dispatch(uri),
          );

          ();
        };

        let update = (~params as _, ~state, ~dispatch as _) => {
          state;
        };

        let dispose = (~params as _, ~state as _) => ();
      });

    let originalUri = (~handle, ~filePath, ~toMsg, client) => {
      OriginalUriSubscription.create({filePath, handle, client})
      |> Isolinear.Sub.map(uri => toMsg(uri));
    };
  };
  type bufferParams = {
    client: Exthost.Client.t,
    buffer: Oni_Core.Buffer.t,
  };

  module BufferSubscription =
    Isolinear.Sub.Make({
      type nonrec msg = [ | `Added];
      type nonrec params = bufferParams;
      type state = {
        didAdd: bool,
        lastFileType: string,
      };

      let name = "Service_Exthost.BufferSubscription";
      let id = params => {
        params.buffer |> Oni_Core.Buffer.getId |> string_of_int;
      };

      let init = (~params, ~dispatch) => {
        let bufferId = Oni_Core.Buffer.getId(params.buffer);

        let fileType =
          params.buffer
          |> Oni_Core.Buffer.getFileType
          |> Oni_Core.Buffer.FileType.toString;

        Log.infof(m => m("Starting buffer subscription for: %d", bufferId));

        fileType |> Internal.activateFileType(~client=params.client);

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
          dispatch(`Added);
          {lastFileType: fileType, didAdd: true};
        | None => {lastFileType: fileType, didAdd: false}
        };
      };

      let update = (~params, ~state, ~dispatch as _) => {
        let newFileType =
          Oni_Core.Buffer.getFileType(params.buffer)
          |> Oni_Core.Buffer.FileType.toString;

        if (state.lastFileType != newFileType && state.didAdd) {
          // Ensure relevant extensions are activated
          newFileType |> Internal.activateFileType(~client=params.client);

          Log.infof(m =>
            m(
              "Updated mode for extension host - old: %s new: %s",
              state.lastFileType,
              newFileType,
            )
          );
          let () =
            Exthost.Request.Documents.acceptModelModeChanged(
              ~uri=Oni_Core.Buffer.getUri(params.buffer),
              ~oldModeId=state.lastFileType,
              ~newModeId=newFileType,
              params.client,
            );
          ();
        };

        {...state, lastFileType: newFileType};
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

  let buffer = (~buffer, ~client, ~toMsg) =>
    BufferSubscription.create({buffer, client}) |> Isolinear.Sub.map(toMsg);

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

  type bufferPositionParams = {
    handle: int,
    client: Exthost.Client.t,
    buffer: Oni_Core.Buffer.t,
    position: Exthost.OneBasedPosition.t,
  };

  let idFromBufferPosition = (~handle, ~buffer, ~position, name) => {
    Exthost.OneBasedPosition.(
      Printf.sprintf(
        "%d-%d-%d-%d.%s",
        handle,
        Oni_Core.Buffer.getId(buffer),
        position.lineNumber,
        position.column,
        name,
      )
    );
  };
  let idFromBufferPositionVersion = (~handle, ~buffer, ~position, name) => {
    Exthost.OneBasedPosition.(
      Printf.sprintf(
        "%d-%d-%d-%d.%d.%s",
        handle,
        Oni_Core.Buffer.getId(buffer),
        position.lineNumber,
        position.column,
        Oni_Core.Buffer.getVersion(buffer),
        name,
      )
    );
  };

  module DefinitionSubscription =
    Isolinear.Sub.Make({
      type nonrec msg = list(Exthost.DefinitionLink.t);
      type nonrec params = bufferPositionParams;

      type state = unit;

      let name = "Service_Exthost.DefinitionSubscription";
      let id = ({handle, buffer, position, _}: bufferPositionParams) =>
        idFromBufferPosition(
          ~handle,
          ~buffer,
          ~position,
          "DefinitionSubscription",
        );

      let init = (~params, ~dispatch) => {
        let promise =
          Exthost.Request.LanguageFeatures.provideDefinition(
            ~handle=params.handle,
            ~resource=Oni_Core.Buffer.getUri(params.buffer),
            ~position=params.position,
            params.client,
          );

        Lwt.on_success(promise, definitionLinks => dispatch(definitionLinks));

        Lwt.on_failure(promise, _ => dispatch([]));

        ();
      };

      let update = (~params as _, ~state, ~dispatch as _) => state;

      let dispose = (~params as _, ~state as _) => {
        ();
      };
    });

  let definition = (~handle, ~buffer, ~position, ~toMsg, client) => {
    let position = position |> Exthost.OneBasedPosition.ofPosition;
    DefinitionSubscription.create({handle, buffer, position, client})
    |> Isolinear.Sub.map(toMsg);
  };

  module DocumentHighlightsSub =
    Isolinear.Sub.Make({
      type nonrec msg = list(Exthost.DocumentHighlight.t);
      type nonrec params = bufferPositionParams;

      type state = unit;

      let name = "Service_Exthost.DocumentHighlightsSubscription";
      let id = ({handle, buffer, position, _}) =>
        idFromBufferPositionVersion(
          ~handle,
          ~buffer,
          ~position,
          "DocumentHighlightsSubscription",
        );

      let init = (~params, ~dispatch) => {
        let promise =
          Exthost.Request.LanguageFeatures.provideDocumentHighlights(
            ~handle=params.handle,
            ~resource=Oni_Core.Buffer.getUri(params.buffer),
            ~position=params.position,
            params.client,
          );

        Lwt.on_success(promise, documentHighlights =>
          dispatch(documentHighlights)
        );

        Lwt.on_failure(promise, _ => dispatch([]));

        ();
      };

      let update = (~params as _, ~state, ~dispatch as _) => state;

      let dispose = (~params as _, ~state as _) => {
        ();
      };
    });

  let documentHighlights = (~handle, ~buffer, ~position, ~toMsg, client) => {
    let position = position |> Exthost.OneBasedPosition.ofPosition;
    DocumentHighlightsSub.create({handle, buffer, position, client})
    |> Isolinear.Sub.map(toMsg);
  };

  type completionParams = {
    handle: int,
    context: Exthost.CompletionContext.t,
    client: Exthost.Client.t,
    buffer: Oni_Core.Buffer.t,
    position: Exthost.OneBasedPosition.t,
  };

  module CompletionSubscription =
    Isolinear.Sub.Make({
      type nonrec msg = result(Exthost.SuggestResult.t, string);
      type nonrec params = completionParams;

      type state = unit;

      let name = "Service_Exthost.CompletionSubscription";
      let id = ({handle, buffer, position, _}: params) =>
        idFromBufferPosition(
          ~handle,
          ~buffer,
          ~position,
          "CompletionSubscription",
        );

      let init = (~params, ~dispatch) => {
        let promise =
          Exthost.Request.LanguageFeatures.provideCompletionItems(
            ~handle=params.handle,
            ~resource=Oni_Core.Buffer.getUri(params.buffer),
            ~position=params.position,
            ~context=params.context,
            params.client,
          );

        Lwt.on_success(promise, suggestResult =>
          dispatch(Ok(suggestResult))
        );

        Lwt.on_failure(promise, exn =>
          dispatch(Error(Printexc.to_string(exn)))
        );

        ();
      };

      let update = (~params as _, ~state, ~dispatch as _) => state;

      let dispose = (~params as _, ~state as _) => {
        ();
      };
    });
  let completionItems =
      (~handle, ~context, ~buffer, ~position, ~toMsg, client) => {
    let position = position |> Exthost.OneBasedPosition.ofPosition;
    CompletionSubscription.create(
      {handle, context, buffer, position, client}: completionParams,
    )
    |> Isolinear.Sub.map(toMsg);
  };

  type completionItemParams = {
    handle: int,
    chainedCacheId: Exthost.ChainedCacheId.t,
    client: Exthost.Client.t,
  };

  module CompletionItemSubscription =
    Isolinear.Sub.Make({
      type nonrec msg = result(Exthost.SuggestItem.t, string);
      type nonrec params = completionItemParams;

      type state = unit;

      let name = "Service_Exthost.CompletionItemSubscription";
      let id = ({handle, chainedCacheId, _}: params) =>
        string_of_int(handle) ++ Exthost.ChainedCacheId.show(chainedCacheId);

      let init = (~params, ~dispatch) => {
        let promise =
          Exthost.Request.LanguageFeatures.resolveCompletionItem(
            ~handle=params.handle,
            ~chainedCacheId=params.chainedCacheId,
            params.client,
          );

        Lwt.on_success(promise, suggestItem => dispatch(Ok(suggestItem)));

        Lwt.on_failure(promise, exn =>
          dispatch(Error(Printexc.to_string(exn)))
        );

        ();
      };

      let update = (~params as _, ~state, ~dispatch as _) => state;

      let dispose = (~params as _, ~state as _) => {
        ();
      };
    });
  let completionItem = (~handle, ~chainedCacheId, ~toMsg, client) => {
    CompletionItemSubscription.create(
      {handle, chainedCacheId, client}: completionItemParams,
    )
    |> Isolinear.Sub.map(toMsg);
  };

  let idFromBufferHandle = (~handle, ~buffer) =>
    Printf.sprintf(
      "%d.%d.%d",
      handle,
      Oni_Core.Buffer.getVersion(buffer),
      Oni_Core.Buffer.getId(buffer),
    );

  type bufferHandleParams = {
    handle: int,
    buffer: Oni_Core.Buffer.t,
    client: Exthost.Client.t,
  };

  module DocumentSymbolsSub =
    Isolinear.Sub.Make({
      type nonrec msg = list(Exthost.DocumentSymbol.t);
      type nonrec params = bufferHandleParams;

      type state = unit;

      let name = "Service_Exthost.DocumentSymbolSubscription";
      let id = ({handle, buffer, _}: params) =>
        idFromBufferHandle(~handle, ~buffer);

      let init = (~params, ~dispatch) => {
        let promise =
          Exthost.Request.LanguageFeatures.provideDocumentSymbols(
            ~handle=params.handle,
            ~resource=Oni_Core.Buffer.getUri(params.buffer),
            params.client,
          );

        Lwt.on_success(promise, documentSymbols => dispatch(documentSymbols));

        Lwt.on_failure(promise, _err => dispatch([]));

        ();
      };

      let update = (~params as _, ~state, ~dispatch as _) => state;

      let dispose = (~params as _, ~state as _) => {
        ();
      };
    });

  let documentSymbols = (~handle, ~buffer, ~toMsg, client) => {
    DocumentSymbolsSub.create({handle, buffer, client})
    |> Isolinear.Sub.map(toMsg);
  };
};
