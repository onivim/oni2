open Oni_Core;

module BufferTracker =
  BufferTracker.Make({});

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
        Oni_Core.Log.perf("exthost.bufferUpdate", () =>
          if (BufferTracker.isTracking(Buffer.getId(buffer))) {
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
          } else {
            ();
              // TODO: Warn
          }
        )
      );

    let modelSaved = (~uri, client, toMsg) => {
      Isolinear.Effect.createWithDispatch(~name="exthost.modelSaved", dispatch => {
        Exthost.Request.Documents.acceptModelSaved(~uri, client);
        dispatch(toMsg());
      });
    };
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
  };

  module Workspace = {
    let change = (~workspace, extHostClient) =>
      Isolinear.Effect.create(~name="exthost.changeWorkspace", () => {
        Exthost.Request.Workspace.acceptWorkspaceData(
          ~workspace,
          extHostClient,
        )
      });
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
        // TODO: How does this compare with an alternative approach to the same ends, like
        // converting from an array back to a list?
        lines |> List.rev |> List.append([""]) |> List.rev;
      };

    maybeFilePath
    |> Option.map(filePath => {
         Log.tracef(m => m("Creating model for filetype: %s", modeId));
         Exthost.ModelAddedDelta.create(
           ~versionId=version,
           ~lines,
           ~modeId,
           ~isDirty=false,
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
        Printf.sprintf(
          "%d:%s",
          params.buffer |> Oni_Core.Buffer.getId,
          // Use the buffer path, too, so that if it changes, we subscribe to the new file
          Buffer.getFilePath(params.buffer)
          |> Option.value(~default="(null)"),
        );
      };

      let init = (~params, ~dispatch) => {
        BufferTracker.startTracking(Oni_Core.Buffer.getId(params.buffer));
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
          BufferTracker.startTracking(Oni_Core.Buffer.getId(params.buffer));
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

      let dispose = (~params, ~state) => {
        BufferTracker.stopTracking(Oni_Core.Buffer.getId(params.buffer));
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

      type state = unit => unit;

      let name = "Service_Exthost.DefinitionSubscription";
      let id = ({handle, buffer, position, _}: bufferPositionParams) =>
        idFromBufferPosition(
          ~handle,
          ~buffer,
          ~position,
          "DefinitionSubscription",
        );

      let init = (~params, ~dispatch) => {
        Revery.Tick.timeout(
          ~name="Timeout.provideDefinition",
          _ => {
            let promise =
              Exthost.Request.LanguageFeatures.provideDefinition(
                ~handle=params.handle,
                ~resource=Oni_Core.Buffer.getUri(params.buffer),
                ~position=params.position,
                params.client,
              );

            Lwt.on_success(promise, definitionLinks =>
              dispatch(definitionLinks)
            );

            Lwt.on_failure(promise, _ => dispatch([]));
          },
          Constants.highPriorityDebounceTime,
        );
      };

      let update = (~params as _, ~state, ~dispatch as _) => state;

      let dispose = (~params as _, ~state as dispose) => {
        dispose();
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

      type state = unit => unit;

      let name = "Service_Exthost.DocumentHighlightsSubscription";
      let id = ({handle, buffer, position, _}) =>
        idFromBufferPositionVersion(
          ~handle,
          ~buffer,
          ~position,
          "DocumentHighlightsSubscription",
        );

      let init = (~params, ~dispatch) => {
        Revery.Tick.timeout(
          ~name="Timeout.provideHighlights",
          () => {
            let promise =
              Exthost.Request.LanguageFeatures.provideDocumentHighlights(
                ~handle=params.handle,
                ~resource=Oni_Core.Buffer.getUri(params.buffer),
                ~position=params.position,
                params.client,
              );

            Lwt.on_success(promise, maybeDocumentHighlights =>
              maybeDocumentHighlights |> Option.value(~default=[]) |> dispatch
            );

            Lwt.on_failure(promise, _ => dispatch([]));
          },
          Constants.highPriorityDebounceTime,
        );
      };

      let update = (~params as _, ~state, ~dispatch as _) => state;

      let dispose = (~params as _, ~state as dispose) => {
        dispose();
      };
    });

  let documentHighlights = (~handle, ~buffer, ~position, ~toMsg, client) => {
    let position = position |> Exthost.OneBasedPosition.ofPosition;
    DocumentHighlightsSub.create({handle, buffer, position, client})
    |> Isolinear.Sub.map(toMsg);
  };

  type codeLensesParams = {
    handle: int,
    eventTick: int,
    client: Exthost.Client.t,
    buffer: Oni_Core.Buffer.t,
    startLine: int, // One-based start line
    stopLine: int // One-based stop line
  };

  let idForCodeLens = (~handle, ~buffer, ~startLine, ~eventTick) => {
    Printf.sprintf(
      "%d-%d-%d:%d-%d",
      handle,
      Oni_Core.Buffer.getId(buffer),
      Oni_Core.Buffer.getVersion(buffer),
      startLine,
      eventTick,
    );
  };

  module CodeLensesSubscription =
    Isolinear.Sub.Make({
      type nonrec msg = result(list(Exthost.CodeLens.lens), string);
      type nonrec params = codeLensesParams;

      type state = {
        maybeCacheId: ref(option(Exthost.CodeLens.List.cacheId)),
        isActive: ref(bool),
        disposeTimeout: unit => unit,
      };

      let name = "Service_Exthost.CodeLensesSubscription";
      let id = ({handle, buffer, startLine, eventTick, _}: params) =>
        idForCodeLens(~handle, ~buffer, ~startLine, ~eventTick);

      let cleanup = (~maybeCacheId, ~params) => {
        maybeCacheId^
        |> Option.iter(cacheId => {
             Exthost.Request.LanguageFeatures.releaseCodeLenses(
               ~handle=params.handle,
               ~cacheId,
               params.client,
             );

             maybeCacheId := None;
           });
      };

      let init = (~params, ~dispatch) => {
        let active = ref(true);
        let maybeCacheId = ref(None);
        let disposeTimeout =
          Revery.Tick.timeout(
            ~name,
            _ => {
              let init =
                Exthost.Request.LanguageFeatures.provideCodeLenses(
                  ~handle=params.handle,
                  ~resource=Oni_Core.Buffer.getUri(params.buffer),
                  params.client,
                );

              let resolveLens = (codeLens: Exthost.CodeLens.lens) =>
                if (! active^) {
                  // If the subscription is over, don't both resolving...
                  Lwt.return(
                    None,
                  );
                } else {
                  switch (codeLens.command) {
                  | Some(_) => Lwt.return(Some(codeLens))
                  | None =>
                    Exthost.Request.LanguageFeatures.resolveCodeLens(
                      ~handle=params.handle,
                      ~codeLens,
                      params.client,
                    )
                  };
                };

              let promise =
                Lwt.bind(
                  init,
                  (initResult: option(Exthost.CodeLens.List.t)) => {
                    initResult
                    |> Option.iter(({cacheId, _}: Exthost.CodeLens.List.t) => {
                         maybeCacheId := cacheId
                       });
                    if (active^) {
                      let unresolvedLenses =
                        initResult
                        |> Option.map((lensResult: Exthost.CodeLens.List.t) =>
                             lensResult.lenses
                           )
                        |> Option.value(~default=[])
                        |> List.filter((lens: Exthost.CodeLens.lens) => {
                             Exthost.OneBasedRange.(
                               {
                                 lens.range.startLineNumber >= params.startLine
                                 && lens.range.startLineNumber
                                 <= params.stopLine;
                               }
                             )
                           });

                      let resolvePromises =
                        unresolvedLenses |> List.map(resolveLens);

                      let join:
                        (
                          list(Exthost.CodeLens.lens),
                          option(Exthost.CodeLens.lens)
                        ) =>
                        list(Exthost.CodeLens.lens) =
                        (acc, cur) => {
                          switch (cur) {
                          | None => acc
                          | Some(lens) => [lens, ...acc]
                          };
                        };

                      Utility.LwtEx.all(~initial=[], join, resolvePromises);
                    } else {
                      cleanup(~maybeCacheId, ~params);
                      Lwt.return([]);
                    };
                  },
                );

              Lwt.on_success(promise, codeLenses => {
                dispatch(Ok(codeLenses))
              });

              Lwt.on_failure(promise, exn => {
                dispatch(Error(Printexc.to_string(exn)))
              });
            },
            Constants.lowPriorityDebounceTime,
          );
        {isActive: active, disposeTimeout, maybeCacheId};
      };

      let update = (~params as _, ~state, ~dispatch as _) => state;

      let dispose = (~params, ~state) => {
        cleanup(~params, ~maybeCacheId=state.maybeCacheId);
        state.isActive := false;
        state.disposeTimeout();
      };
    });

  let codeLenses =
      (~handle, ~eventTick, ~buffer, ~startLine, ~stopLine, ~toMsg, client) => {
    let startLine = EditorCoreTypes.LineNumber.toOneBased(startLine);
    let stopLine = EditorCoreTypes.LineNumber.toOneBased(stopLine);
    CodeLensesSubscription.create(
      {handle, buffer, client, startLine, stopLine, eventTick}: codeLensesParams,
    )
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

      type state = {
        isDisposed: ref(bool),
        cacheId: ref(option(Exthost.SuggestResult.cacheId)),
      };

      let name = "Service_Exthost.CompletionSubscription";
      let id = ({handle, buffer, position, _}: params) =>
        idFromBufferPosition(
          ~handle,
          ~buffer,
          ~position,
          "CompletionSubscription",
        );

      let cleanupCache =
          (~params, ~cacheId: ref(option(Exthost.SuggestResult.cacheId))) => {
        cacheId^
        |> Option.iter(cacheId => {
             Exthost.Request.LanguageFeatures.releaseCompletionItems(
               ~handle=params.handle,
               ~cacheId,
               params.client,
             )
           });
      };

      let init = (~params, ~dispatch) => {
        let isDisposed = ref(false);
        let cacheId = ref(None);
        let promise =
          Exthost.Request.LanguageFeatures.provideCompletionItems(
            ~handle=params.handle,
            ~resource=Oni_Core.Buffer.getUri(params.buffer),
            ~position=params.position,
            ~context=params.context,
            params.client,
          );

        Lwt.on_success(
          promise,
          suggestResult => {
            cacheId := suggestResult.cacheId;
            if (isDisposed^) {
              cleanupCache(~params, ~cacheId);
            } else {
              dispatch(Ok(suggestResult));
            };
          },
        );

        Lwt.on_failure(promise, exn =>
          dispatch(Error(Printexc.to_string(exn)))
        );

        {isDisposed, cacheId};
      };

      let update = (~params as _, ~state, ~dispatch as _) => state;

      let dispose = (~params, ~state) => {
        state.isDisposed := true;
        cleanupCache(~params, ~cacheId=state.cacheId);
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

  type signatureHelpParams = {
    handle: int,
    context: Exthost.SignatureHelp.RequestContext.t,
    client: Exthost.Client.t,
    buffer: Oni_Core.Buffer.t,
    position: Exthost.OneBasedPosition.t,
  };

  module SignatureHelpSubscription =
    Isolinear.Sub.Make({
      type nonrec msg =
        result(option(Exthost.SignatureHelp.Response.t), string);
      type nonrec params = signatureHelpParams;

      type state = {
        isActive: ref(bool),
        cacheId: ref(option(Exthost.SignatureHelp.cacheId)),
      };

      let cleanup = (~params, ~cacheId) => {
        cacheId^
        |> Option.iter(cacheId => {
             Exthost.Request.LanguageFeatures.releaseSignatureHelp(
               ~handle=params.handle,
               ~cacheId,
               params.client,
             )
           });
      };

      let name = "Service_Exthost.SignatureHelpSubscription";
      let id = ({handle, buffer, position, _}: params) =>
        idFromBufferPosition(~handle, ~buffer, ~position, "SignatureHelp");

      let init = (~params, ~dispatch) => {
        let isActive = ref(true);
        let cacheId = ref(None);
        let promise =
          Exthost.Request.LanguageFeatures.provideSignatureHelp(
            ~handle=params.handle,
            ~resource=Oni_Core.Buffer.getUri(params.buffer),
            ~position=params.position,
            ~context=params.context,
            params.client,
          );

        Lwt.on_success(
          promise,
          (maybeSigHelpResult: option(Exthost.SignatureHelp.Response.t)) => {
            maybeSigHelpResult
            |> Option.iter(suggestResult => {
                 cacheId :=
                   Some(
                     Exthost.SignatureHelp.Response.(suggestResult.cacheId),
                   )
               });

            if (isActive^) {
              dispatch(Ok(maybeSigHelpResult));
            } else {
              cleanup(~params, ~cacheId);
            };
          },
        );

        Lwt.on_failure(promise, exn =>
          dispatch(Error(Printexc.to_string(exn)))
        );

        {isActive, cacheId};
      };

      let update = (~params as _, ~state, ~dispatch as _) => state;

      let dispose = (~params, ~state) => {
        state.isActive := false;
        cleanup(~params, ~cacheId=state.cacheId);
      };
    });
  let signatureHelp = (~handle, ~context, ~buffer, ~position, ~toMsg, client) => {
    let position = position |> Exthost.OneBasedPosition.ofPosition;
    SignatureHelpSubscription.create(
      {handle, context, buffer, position, client}: signatureHelpParams,
    )
    |> Isolinear.Sub.map(toMsg);
  };

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

      type state = unit => unit;

      let name = "Service_Exthost.DocumentSymbolSubscription";
      let id = ({handle, buffer, _}: params) =>
        idFromBufferHandle(~handle, ~buffer);

      let init = (~params, ~dispatch) => {
        Revery.Tick.timeout(
          ~name="Timeout.documentSymbols",
          _ => {
            let promise =
              Exthost.Request.LanguageFeatures.provideDocumentSymbols(
                ~handle=params.handle,
                ~resource=Oni_Core.Buffer.getUri(params.buffer),
                params.client,
              );

            Lwt.on_success(promise, maybeDocumentSymbols =>
              maybeDocumentSymbols |> Option.value(~default=[]) |> dispatch
            );

            Lwt.on_failure(promise, _err => dispatch([]));
          },
          Constants.lowPriorityDebounceTime,
        );
      };

      let update = (~params as _, ~state, ~dispatch as _) => state;

      let dispose = (~params as _, ~state as dispose) => {
        dispose();
      };
    });

  let documentSymbols = (~handle, ~buffer, ~toMsg, client) => {
    DocumentSymbolsSub.create({handle, buffer, client})
    |> Isolinear.Sub.map(toMsg);
  };
};
