open Oni_Core;
open Oni_Core.Utility;
open Feature_Editor;
open Exthost;

module Log = (val Log.withNamespace("Oni2.Feature.Exthost"));

module Internal = {
  let prefix = "onivim.editor:";
  let prefixLength = String.length(prefix);
  let getVscodeEditorId = editorId =>
    "onivim.editor:" ++ string_of_int(editorId);

  let editorIdFromVscodeEditorId = (vscodeEditorId: string) => {
    String.sub(
      vscodeEditorId,
      prefixLength,
      String.length(vscodeEditorId) - prefixLength,
    )
    |> int_of_string;
  };

  let getExthostSelectionFromEditor = (editor: Feature_Editor.Editor.t) => {
    Feature_Editor.Editor.(
      {
        let byteRange =
          switch (Editor.selections(editor)) {
          | [] =>
            let position = Editor.getPrimaryCursorByte(editor);
            EditorCoreTypes.ByteRange.{start: position, stop: position};

          | [selection, ..._] => VisualRange.(selection.range)
          };

        editor
        |> byteRangeToCharacterRange(byteRange)
        |> Option.map(range => {
             EditorCoreTypes.(
               EditorCoreTypes.CharacterRange.(
                 {
                   let startColumn = range.start.character;
                   let stopColumn = range.stop.character;
                   let startLine = range.start.line;
                   let stopLine = range.stop.line;

                   let selectionStartColumn =
                     (startColumn |> CharacterIndex.toInt) + 1;

                   // If the start - end of the selection are the same, use startColumn.
                   // Otherwise, for the selection, we need to add 2:
                   // - Add 1 to move from zero-based to one-based characters for the extension host
                   // - Add 1 because the selection is inclusive in Vim, but exclusive in the extension host API
                   let positionColumn =
                     startLine == stopLine && startColumn == stopColumn
                       ? selectionStartColumn
                       : (stopColumn |> CharacterIndex.toInt) + 2;
                   [
                     Exthost.Selection.{
                       selectionStartLineNumber:
                         startLine |> LineNumber.toOneBased,
                       selectionStartColumn,
                       positionLineNumber: stopLine |> LineNumber.toOneBased,
                       positionColumn,
                     },
                   ];
                 }
               )
             )
           })
        |> Option.value(~default=[]);
      }
    );
  };

  let editorToAddData:
    (IntMap.t(Buffer.t), Editor.t) => option(TextEditor.AddData.t) =
    (bufferMap, editor) => {
      let maybeBuffer =
        bufferMap
        |> IntMap.find_opt(Feature_Editor.Editor.getBufferId(editor));

      let indentationSettings: IndentationSettings.t =
        maybeBuffer
        |> Option.map(Buffer.getIndentation)
        |> Option.value(~default=IndentationSettings.default);

      maybeBuffer
      |> OptionEx.flatMap(Oni_Core.Buffer.getFilePath)
      |> Option.map(Uri.fromPath)
      |> Option.map(uri => {
           let id = getVscodeEditorId(Feature_Editor.Editor.getId(editor));
           open TextEditor;

           let options =
             ResolvedConfiguration.{
               tabSize: indentationSettings.tabSize,
               indentSize: indentationSettings.size,
               insertSpaces:
                 indentationSettings.mode == IndentationSettings.Spaces
                   ? 1 : 0,
               cursorStyle: CursorStyle.Solid,
               lineNumbers: LineNumbersStyle.On,
             };

           let selections = getExthostSelectionFromEditor(editor);

           AddData.{id, documentUri: uri, options, selections};
         });
    };
};

[@deriving show]
type msg =
  | Noop
  | DocumentAdded({uri: Oni_Core.Uri.t})
  | ExthostDocuments({
      msg: Exthost.Msg.Documents.msg,
      resolver: [@opaque] Lwt.u(Exthost.Reply.t),
    })
  | ExthostTextEditors({
      msg: Exthost.Msg.TextEditors.msg,
      resolver: [@opaque] Lwt.u(Exthost.Reply.t),
    })
  | ApplyTextEditSuccess({resolver: [@opaque] Lwt.u(Exthost.Reply.t)})
  | ApplyTextEditFailure({
      errorMessage: string,
      resolver: [@opaque] Lwt.u(Exthost.Reply.t),
    })
  | Initialized;

module Msg = {
  let document = (msg, resolver) => {
    ExthostDocuments({msg, resolver});
  };

  let textEditors = (msg, resolver) => {
    ExthostTextEditors({msg, resolver});
  };

  let initialized = Initialized;
};

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

type model = {
  isInitialized: bool,
  pendingResolutions: StringMap.t(list(Lwt.u(Exthost.Reply.t))),
};

let initial = {pendingResolutions: StringMap.empty, isInitialized: false};

let isInitialized = ({isInitialized, _}) => isInitialized;

module Effects = {
  let resolve = resolver => {
    Isolinear.Effect.create(~name="resolve", () => {
      Lwt.wakeup(resolver, Reply.okEmpty)
    });
  };

  let resolveFailure = (~msg, resolver) => {
    Isolinear.Effect.create(~name="resolve", () => {
      Lwt.wakeup(resolver, Reply.error(msg))
    });
  };
};

let update = (~buffers, ~editors, msg: msg, model) =>
  switch (msg) {
  | Noop => (model, Nothing)

  | DocumentAdded({uri}) =>
    let filePath = Oni_Core.Uri.toFileSystemPath(uri);
    let eff = Effect(Isolinear.Effect.none);
    // TODO: Fix up
    //  let eff = switch(StringMap.find_opt(filePath, model.pendingResolutions))
    //  {
    //    | None => Nothing
    //    | Some(resolvers) => Effect(resolvers
    //    |> List.map(Effects.resolve)
    //    |> Isolinear.Effect.batch);
    //  }

    let pendingResolutions =
      StringMap.remove(filePath, model.pendingResolutions);

    ({...model, pendingResolutions}, eff);

  | ExthostDocuments({msg, resolver}) =>
    switch (msg) {
    | Exthost.Msg.Documents.TryOpenDocument({uri}) =>
      let filePath = Oni_Core.Uri.toFileSystemPath(uri);
      // TODO: Hook this up end-to-end
      //      let eff = Service_Vim.Effects.loadBuffer(
      //        ~filePath,
      //        (~bufferId)=> Noop
      //      );
      let eff = Effect(Effects.resolve(resolver));
      let pendingResolutions =
        model.pendingResolutions
        |> StringMap.update(
             filePath,
             fun
             | None => Some([resolver])
             | Some(cur) => Some([resolver, ...cur]),
           );
      ({...model, pendingResolutions}, eff);

    | Exthost.Msg.Documents.TrySaveDocument(_) =>
      Log.warn("TrySaveDocument is not yet implemented.");
      (model, Effect(Effects.resolve(resolver)));

    | Exthost.Msg.Documents.TryCreateDocument(_) =>
      Log.warn("TryCreateDocument is not yet implemented.");
      (model, Effect(Effects.resolve(resolver)));
    }

  | ApplyTextEditSuccess({resolver}) => (
      model,
      Effect(Effects.resolve(resolver)),
    )

  | ApplyTextEditFailure({errorMessage, resolver}) => (
      model,
      Effect(Effects.resolveFailure(~msg=errorMessage, resolver)),
    )

  | ExthostTextEditors({msg, resolver}) =>
    switch (msg) {
    | TryApplyEdits({id, modelVersionId, edits}) =>
      // Find buffer id for editor
      let eff =
        editors
        |> List.filter(editor =>
             Feature_Editor.Editor.getId(editor)
             == Internal.editorIdFromVscodeEditorId(id)
           )
        |> Utility.ListEx.nth_opt(0)
        |> Option.map(Feature_Editor.Editor.getBufferId)
        |> Utility.OptionEx.flatMap(bufferId =>
             Feature_Buffers.get(bufferId, buffers)
           )
        |> Option.map(buffer => {
             let bufferId = Oni_Core.Buffer.getId(buffer);
             let vimEdits =
               edits
               |> List.map(
                    Service_Vim.EditConverter.extHostSingleEditToVimEdit(
                      ~buffer,
                    ),
                  );
             let toMsg = (
               fun
               | Error(msg) =>
                 ApplyTextEditFailure({resolver, errorMessage: msg})
               | Ok(_) => ApplyTextEditSuccess({resolver: resolver})
             );

             Effect(
               Service_Vim.Effects.applyEdits(
                 ~shouldAdjustCursors=true,
                 ~bufferId,
                 ~version=modelVersionId,
                 ~edits=vimEdits,
                 toMsg,
               ),
             );
           })
        |> Option.value(
             ~default=
               Effect(
                 EffectEx.value(
                   ~name="ApplyTextEditFailure",
                   ApplyTextEditFailure({
                     resolver,
                     errorMessage: "Unable to get buffer for editor: " ++ id,
                   }),
                 ),
               ),
           );
      (model, eff);
    }

  | Initialized => ({...model, isInitialized: true}, Nothing)
  };

let subscription =
    (
      ~buffers: Feature_Buffers.model,
      ~editors,
      ~activeEditorId,
      ~client,
      model,
    ) =>
  if (isInitialized(model)) {
    let visibleBuffers: list(Oni_Core.Buffer.t) =
      editors
      |> List.map(Feature_Editor.Editor.getBufferId)
      |> List.filter_map(bufferId => Feature_Buffers.get(bufferId, buffers));

    let bufferMap =
      visibleBuffers
      |> List.fold_left(
           (acc, curr) => {
             let id = Buffer.getId(curr);
             IntMap.add(id, curr, acc);
           },
           IntMap.empty,
         );

    let tryOpenedBuffers: list(Oni_Core.Buffer.t) =
      buffers
      |> Feature_Buffers.filter(buf =>
           StringMap.find_opt(
             Oni_Core.Buffer.getUri(buf) |> Oni_Core.Uri.toFileSystemPath,
             model.pendingResolutions,
           )
           |> Option.is_some
         )
      |> List.filter(buffer =>
           IntMap.find_opt(Oni_Core.Buffer.getId(buffer), bufferMap)
           |> Option.is_none
         );

    let bufferSubscriptions =
      visibleBuffers
      |> List.filter(buf => !Feature_Buffers.isLargeFile(buffers, buf))
      |> List.map(buffer => {
           Service_Exthost.Sub.buffer(
             ~buffer,
             ~client,
             ~toMsg=
               fun
               | `Added =>
                 DocumentAdded({uri: buffer |> Oni_Core.Buffer.getUri}),
           )
         })
      |> Isolinear.Sub.batch;

    let tryOpenedBufferSubscriptions =
      tryOpenedBuffers
      |> List.filter(buf => !Feature_Buffers.isLargeFile(buffers, buf))
      |> List.map(buffer => {
           Service_Exthost.Sub.buffer(
             ~buffer,
             ~client,
             ~toMsg=
               fun
               | `Added =>
                 DocumentAdded({uri: buffer |> Oni_Core.Buffer.getUri}),
           )
         })
      |> Isolinear.Sub.batch;

    let editorAddDataAndSelection =
      editors |> List.filter_map(Internal.editorToAddData(bufferMap));

    let editorSubscriptions =
      editorAddDataAndSelection
      |> List.map(editorAddData => {
           Service_Exthost.Sub.editor(~editor=editorAddData, ~client)
         })
      |> Isolinear.Sub.batch
      |> Isolinear.Sub.map(() => Noop);

    let activeEditorSubscription =
      switch (activeEditorId) {
      | None => Isolinear.Sub.none
      | Some(editorId) =>
        Service_Exthost.Sub.activeEditor(
          ~activeEditorId=Internal.getVscodeEditorId(editorId),
          ~client,
        )
        |> Isolinear.Sub.map(() => Noop)
      };

    Isolinear.Sub.batch([
      bufferSubscriptions,
      tryOpenedBufferSubscriptions,
      editorSubscriptions,
      activeEditorSubscription,
    ]);
  } else {
    Isolinear.Sub.none;
  };
