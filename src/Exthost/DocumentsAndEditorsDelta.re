open Oni_Core;

type t = {
  removedDocuments: list(Uri.t),
  addedDocuments: list(ModelAddedDelta.t),
  removedEditors: list(string),
  addedEditors: list(TextEditor.AddData.t),
  newActiveEditor: option(string),
};

let create =
    (
      ~removedDocuments=[],
      ~addedDocuments=[],
      ~removedEditors=[],
      ~addedEditors=[],
      ~newActiveEditor=None,
      (),
    ) => {
  removedDocuments,
  addedDocuments,
  removedEditors,
  addedEditors,
  newActiveEditor,
};

let encode =
    (
      {
        removedDocuments,
        addedDocuments,
        removedEditors,
        addedEditors,
        newActiveEditor,
      },
    ) =>
  Json.Encode.(
    obj([
      ("removedDocuments", removedDocuments |> list(Uri.encode)),
      ("addedDocuments", addedDocuments |> list(ModelAddedDelta.encode)),
      ("removedEditors", removedEditors |> list(string)),
      ("addedEditors", addedEditors |> list(TextEditor.AddData.encode)),
      ("newActiveEditor", newActiveEditor |> nullable(string)),
    ])
  );
