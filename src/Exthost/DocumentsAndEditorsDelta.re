open Oni_Core;

[@deriving yojson({strict: false})]
type t = {
  removedDocuments: list(Uri.t),
  addedDocuments: list(ModelAddedDelta.t),
  removedEditors: list(string),
  addedEditors: list(string),
};

let create = (~removedDocuments, ~addedDocuments) => {
  removedDocuments,
  addedDocuments,
  removedEditors: [],
  addedEditors: [],
};
