open Oni_Core;
open Oni_Core.Utility;
open Feature_Editor;
open Exthost;

module Internal = {
  let getVscodeEditorId = editorId =>
    "onivim.editor:" ++ string_of_int(editorId);

  let editorToAddData:
    (IntMap.t(Buffer.t), Editor.t) => option(TextEditor.AddData.t) =
    (bufferMap, editor) => {
      None;
    };
};

let subscription = (~buffers, ~editors, ~activeEditorId, ~client) => {
  let bufferMap =
    buffers
    |> List.fold_left(
         (acc, curr) => {
           let id = Buffer.getId(curr);
           IntMap.add(id, curr, acc);
         },
         IntMap.empty,
       );

  let bufferSubscriptions =
    buffers
    |> List.map(buffer => {Service_Exthost.Sub.buffer(~buffer, ~client)})
    |> Isolinear.Sub.batch;

  let editors =
    editors
    |> List.map(Internal.editorToAddData(bufferMap))
    |> OptionEx.values;

  let editorSubscriptions =
    editors
    |> List.map(editor => {Service_Exthost.Sub.editor(~editor, ~client)})
    |> Isolinear.Sub.batch;

  let activeEditorSubscription =
    switch (activeEditorId) {
    | None => Isolinear.Sub.none
    | Some(editorId) =>
      Service_Exthost.Sub.activeEditor(
        ~activeEditorId=Internal.getVscodeEditorId(editorId),
        ~client,
      )
    };

  Isolinear.Sub.batch([
    bufferSubscriptions,
    editorSubscriptions,
    activeEditorSubscription,
  ]);
};
