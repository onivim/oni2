open Oni_Core;

module IconPath = {
  type t =
    | IconId({iconId: string})
    | Uri({uri: Oni_Core.Uri.t})
    | LightDarkUri({
        light: Oni_Core.Uri.t,
        dark: Oni_Core.Uri.t,
      });
};

module EntryMetadata = {
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
    type t = {
      overwrite: bool,
      ignoreIfNotExists: bool,
      ignoreIfExists: bool,
      recursive: bool,
    };

    let decode =
      Json.Decode.(
        obj(({field, _}) =>
          {
            overwrite: field.withDefault("overwrite", false, bool),
            ignoreIfNotExists:
              field.withDefault("ignoreIfNotExists", false, bool),
            ignoreIfExists: field.withDefault("ignoreIfExists", false, bool),
            recursive: field.withDefault("recursive", false, bool),
          }
        )
      );

    let default = {
      overwrite: false,
      ignoreIfNotExists: false,
      ignoreIfExists: false,
      recursive: false,
    };
  };

  type t = {
    oldUri: option(Oni_Core.Uri.t),
    newUri: option(Oni_Core.Uri.t),
    options: option(Options.t),
    metadata: option(EntryMetadata.t),
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          oldUri: field.optional("oldUri", Oni_Core.Uri.decode),
          newUri: field.optional("newUri", Oni_Core.Uri.decode),
          options: field.optional("options", Options.decode),
          metadata: field.optional("metadata", EntryMetadata.decode),
        }
      )
    );
};

module SingleEdit = {
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
  type t = {
    resource: Oni_Core.Uri.t,
    edit: SingleEdit.t,
    modelVersionId: option(int),
    metadata: option(EntryMetadata.t),
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          resource: field.required("resource", Oni_Core.Uri.decode),
          edit: field.required("edit", SingleEdit.decode),
          modelVersionId: field.optional("modelVersionId", int),
          metadata: field.optional("metadata", EntryMetadata.decode),
        }
      )
    );
};

type edit =
  | File(FileEdit.t)
  | Text(TextEdit.t);

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
