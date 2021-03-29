open Oni_Core;

module EditType = {
  type t =
    | File
    | Text;
  // TODO:
  // | Cell;

  let ofInt =
    fun
    | 1 => Some(File)
    | 2 => Some(Text)
    | _ => None;

  let toString =
    fun
    | File => "File"
    | Text => "Text";

  let decode =
    Json.Decode.(
      int
      |> map(ofInt)
      |> and_then(
           fun
           | Some(v) => succeed(v)
           | None => fail("Unknown edit type"),
         )
    );

  let decodeFile =
    Json.Decode.(
      decode
      |> and_then(
           fun
           | File => succeed(File)
           | _ => fail("Not a file type"),
         )
    );

  let decodeText =
    Json.Decode.(
      decode
      |> and_then(
           fun
           | Text => succeed(Text)
           | _ => fail("Not a text type"),
         )
    );
};

module IconPath = {
  [@deriving show]
  type t =
    | IconId({iconId: string})
    | Uri({uri: Oni_Core.Uri.t})
    | LightDarkUri({
        light: Oni_Core.Uri.t,
        dark: Oni_Core.Uri.t,
      });
};

module EntryMetadata = {
  [@deriving show]
  type t = {
    needsConfirmation: bool,
    label: string,
    description: option(string),
    iconPath: option(IconPath.t),
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          needsConfirmation:
            field.withDefault("needsConfirmation", false, bool),
          label: field.required("label", string),
          description: field.optional("description", string),
          // TODO:
          iconPath: None,
        }
      )
    );
};

module FileEdit = {
  module Options = {
    [@deriving show]
    type t = {
      overwrite: bool,
      ignoreIfNotExists: bool,
      ignoreIfExists: bool,
      recursive: bool,
    };

    let decode =
      Json.Decode.(
        obj(({field, _}) => {
          {
            overwrite: field.withDefault("overwrite", false, bool),
            ignoreIfNotExists:
              field.withDefault("ignoreIfNotExists", false, bool),
            ignoreIfExists: field.withDefault("ignoreIfExists", false, bool),
            recursive: field.withDefault("recursive", false, bool),
          }
        })
      );

    let default = {
      overwrite: false,
      ignoreIfNotExists: false,
      ignoreIfExists: false,
      recursive: false,
    };
  };

  [@deriving show]
  type t = {
    oldUri: option(Oni_Core.Uri.t),
    newUri: option(Oni_Core.Uri.t),
    options: option(Options.t),
    metadata: option(EntryMetadata.t),
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) => {
        let _type = field.required("_type", EditType.decodeFile);
        {
          oldUri: field.optional("oldUri", Oni_Core.Uri.decode),
          newUri: field.optional("newUri", Oni_Core.Uri.decode),
          options: field.optional("options", Options.decode),
          metadata: field.optional("metadata", EntryMetadata.decode),
        };
      })
    );
};

module SingleEdit = {
  [@deriving show]
  type t = {
    range: OneBasedRange.t,
    text: string,
    // eol?
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          range: field.required("range", OneBasedRange.decode),
          text: field.required("text", string),
          // TODO: EOL
        }
      )
    );
};

module TextEdit = {
  [@deriving show]
  type t = {
    resource: Oni_Core.Uri.t,
    edit: SingleEdit.t,
    modelVersionId: option(int),
    metadata: option(EntryMetadata.t),
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) => {
        let _type = field.required("_type", EditType.decodeText);

        {
          resource: field.required("resource", Oni_Core.Uri.decode),
          edit: field.required("edit", SingleEdit.decode),
          modelVersionId: field.optional("modelVersionId", int),
          metadata: field.optional("metadata", EntryMetadata.decode),
        };
      })
    );
};

[@deriving show]
type edit =
  | File(FileEdit.t)
  | Text(TextEdit.t);

[@deriving show]
type t = {
  edits: list(edit),
  rejectReason: option(string),
};

module Decode = {
  open Json.Decode;

  let fileEdit = FileEdit.decode |> map(fileEdit => File(fileEdit));

  let textEdit = TextEdit.decode |> map(textEdit => Text(textEdit));

  let edit = one_of([("fileEdit", fileEdit), ("textEdit", textEdit)]);
};

let decode =
  Json.Decode.(
    obj(({field, _}) =>
      {
        edits: field.withDefault("edits", [], list(Decode.edit)),
        rejectReason: field.optional("rejectReason", string),
      }
    )
  );
