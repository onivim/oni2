open Oni_Core;

[@deriving show]
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
  Json.Encode.
    // Differentation between `null` and `undefined` is important for the `newActiveEditor` field:
    // https://github.com/onivim/vscode-exthost/blob/c7df89c1cf0087ca5decaf8f6d4c0fd0257a8b7a/src/vs/workbench/api/common/extHostDocumentsAndEditors.ts#L129
    (
      {
        let maybeNewActiveEditor =
          if (newActiveEditor == None) {
            [];
          } else {
            [("newActiveEditor", newActiveEditor |> nullable(string))];
          };

        obj(
          [
            ("removedDocuments", removedDocuments |> list(Uri.encode)),
            (
              "addedDocuments",
              addedDocuments |> list(ModelAddedDelta.encode),
            ),
            ("removedEditors", removedEditors |> list(string)),
            (
              "addedEditors",
              addedEditors |> list(TextEditor.AddData.encode),
            ),
          ]
          @ maybeNewActiveEditor,
        );
      }
    );
