open Oni_Core;
module Folder = {
  type t = {
    uri: Uri.t,
    name: string,
    index: int,
  };

  let decode = {
    Json.Decode.(
      obj(({field, _}) =>
        {
          uri: field.required("uri", Uri.decode),
          name: field.required("name", string),
          index: field.required("index", int),
        }
      )
    );
  };

  let encode = folder =>
    Json.Encode.(
      obj([
        ("uri", folder.uri |> Uri.encode),
        ("name", folder.name |> string),
        ("index", folder.index |> int),
      ])
    );
};

type t = {
  folders: list(Folder.t),
  id: string,
  name: string,
  configuration: option(Uri.t),
  isUntitled: bool,
};

let decode = {
  Json.Decode.(
    obj(({field, _}) =>
      {
        folders: field.withDefault("folders", [], list(Folder.decode)),
        id: field.required("id", string),
        name: field.required("name", string),
        configuration: field.optional("configuration", Uri.decode),
        isUntitled: field.withDefault("isUntitled", false, bool),
      }
    )
  );
};

let encode = workspace =>
  Json.Encode.(
    obj([
      ("folders", workspace.folders |> list(Folder.encode)),
      ("id", workspace.id |> string),
      ("name", workspace.name |> string),
      ("configuration", workspace.configuration |> nullable(Uri.encode)),
      ("isUntitled", workspace.isUntitled |> bool),
    ])
  );

module Internal = {
  let hash =
    fun
    | "global" => "global"
    | value => value |> Hashtbl.hash |> Printf.sprintf("%x");
};

let fromUri = (~name, uri) => {
  id: Internal.hash(Uri.toString(uri)),
  name,
  folders: [{uri, name, index: 0}],
  isUntitled: false,
  configuration: None,
};

let fromPath = path => {
  id: Internal.hash(path),
  name: path,
  configuration: None,
  isUntitled: false,
  folders: [{uri: Uri.fromPath(path), name: path, index: 0}],
};
