open Oni_Core;
open Oni_Core.Utility;
open Feature_Editor;
open Exthost;

module Log = (val Log.withNamespace("Oni2.Feature.Exthost"));

module Internal = {
  let getVscodeEditorId = editorId =>
    "onivim.editor:" ++ string_of_int(editorId);

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

           AddData.{id, documentUri: uri, options};
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
  | Initialized;

module Msg = {
  let document = (msg, resolver) => {
    ExthostDocuments({msg, resolver});
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
};

let update = (msg: msg, model) =>
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

  | Initialized => ({...model, isInitialized: true}, Nothing)
  };

let subscription =
    (
      ~buffers: Feature_Buffers.model,
      ~editors,
      ~activeEditorId,
      ~client,
      model,
    ) => {
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

  let editors =
    editors
    |> List.map(Internal.editorToAddData(bufferMap))
    |> OptionEx.values;

  let editorSubscriptions =
    editors
    |> List.map(editor => {Service_Exthost.Sub.editor(~editor, ~client)})
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
};
