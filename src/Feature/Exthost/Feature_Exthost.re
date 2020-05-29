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
      let maybeBuffer =
        bufferMap
        |> IntMap.find_opt(Feature_Editor.Editor.getBufferId(editor));

      let indentationSettings: IndentationSettings.t =
        maybeBuffer
        |> OptionEx.flatMap(Buffer.getIndentation)
        |> Option.value(~default=IndentationSettings.default);

      maybeBuffer
      |> OptionEx.flatMap(Oni_Core.Buffer.getFilePath)
      |> Option.map(Uri.fromPath)
      |> Option.map(uri => {
           let id = getVscodeEditorId(editor.editorId);
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
