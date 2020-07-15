open Oni_Core;
module FileSystemProviderCapabilities = {
  type capability = [
    | `FileReadWrite
    | `FileOpenReadWriteClose
    | `FileReadStream
    | `FileFolderCopy
    | `PathCaseSensitive
    | `Readonly
    | `Trash
  ];

  // Must be kept in sync with this:
  // https://github.com/onivim/vscode-exthost/blob/c7df89c1cf0087ca5decaf8f6d4c0fd0257a8b7a/src/vs/platform/files/common/files.ts#L234
  let capabilityToInt =
    fun
    | `FileReadWrite => 1 lsl 1
    | `FileOpenReadWriteClose => 1 lsl 2
    | `FileReadStream => 1 lsl 4
    | `FileFolderCopy => 1 lsl 3
    | `PathCaseSensitive => 1 lsl 10
    | `Readonly => 1 lsl 11
    | `Trash => 1 lsl 12;

  [@deriving show]
  type t = int;

  let test = (capability, capabilities) => {
    let mask = capabilityToInt(capability);
    mask land capabilities == mask;
  };

  let decode = Json.Decode.int;
};

module FileChangeType = {
  [@deriving show]
  type t =
    | Updated
    | Added
    | Deleted;

  // Must be kept in sync with:
  // https://github.com/onivim/vscode-exthost/blob/master/src/vs/platform/files/common/files.ts#L234
  let ofInt =
    fun
    | 0 => Some(Updated)
    | 1 => Some(Added)
    | 2 => Some(Deleted)
    | _ => None;

  let toInt =
    fun
    | Updated => 0
    | Added => 1
    | Deleted => 2;

  let decode = {
    Json.Decode.(
      {
        int
        |> and_then(i => {
             switch (ofInt(i)) {
             | Some(fileChange) => succeed(fileChange)
             | None =>
               fail("Unexpected FileChangeType: " ++ string_of_int(i))
             }
           });
      }
    );
  };
};

module FileChange = {
  [@deriving show]
  type t = {
    resource: Uri.t,
    changeType: FileChangeType.t,
  };

  let decode = {
    Json.Decode.(
      {
        obj(({field, _}) =>
          {
            resource: field.required("resource", Uri.decode),
            changeType: field.required("type", FileChangeType.decode),
          }
        );
      }
    );
  };
};

module FileType = {
  [@deriving show]
  type t =
    | Unknown
    | File
    | Directory
    | SymbolicLink;

  // Must be kept in sync with:
  // https://github.com/onivim/vscode-exthost/blob/c7df89c1cf0087ca5decaf8f6d4c0fd0257a8b7a/src/vs/platform/files/common/files.ts#L206
  let ofInt =
    fun
    | 0 => Some(Unknown)
    | 1 => Some(File)
    | 2 => Some(Directory)
    | 64 => Some(SymbolicLink)
    | _ => None;

  let toInt =
    fun
    | Unknown => 0
    | File => 1
    | Directory => 2
    | SymbolicLink => 64;

  let decode = {
    Json.Decode.(
      {
        int
        |> and_then(i => {
             switch (ofInt(i)) {
             | Some(fileType) => succeed(fileType)
             | None => fail("Unexpected FileType: " ++ string_of_int(i))
             }
           });
      }
    );
  };

  let encode = fileType => Json.Encode.int(toInt(fileType));
};

module FileOverwriteOptions = {
  [@deriving show]
  type t = {overwrite: bool};

  let decode = {
    Json.Decode.(
      obj(({field, _}) => {overwrite: field.required("overwrite", bool)})
    );
  };
};

module FileWriteOptions = {
  [@deriving show]
  type t = {
    overwrite: bool,
    create: bool,
  };

  let decode = {
    Json.Decode.(
      obj(({field, _}) =>
        {
          overwrite: field.required("overwrite", bool),
          create: field.required("create", bool),
        }
      )
    );
  };
};

module FileOpenOptions = {
  [@deriving show]
  type t = {create: bool};

  let decode = {
    Json.Decode.(
      obj(({field, _}) => {create: field.required("create", bool)})
    );
  };
};

module FileDeleteOptions = {
  [@deriving show]
  type t = {
    recursive: bool,
    useTrash: bool,
  };

  let decode = {
    Json.Decode.(
      obj(({field, _}) =>
        {
          recursive: field.required("recursive", bool),
          useTrash: field.required("useTrash", bool),
        }
      )
    );
  };
};

module StatResult = {
  [@deriving show]
  type t = {
    fileType: FileType.t,
    mtime: int,
    ctime: int,
    size: int,
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          fileType: field.required("type", FileType.decode),
          mtime: field.required("mtime", int),
          ctime: field.required("ctime", int),
          size: field.required("size", int),
        }
      )
    );

  let encode = statResult => {
    Json.Encode.(
      obj([
        ("type", statResult.fileType |> FileType.encode),
        ("mtime", statResult.mtime |> int),
        ("ctime", statResult.ctime |> int),
        ("size", statResult.size |> int),
      ])
    );
  };
};

module ReadDirResult = {
  [@deriving show]
  type t = (string, FileType.t);

  let encode = ((name, fileType): t) =>
    Json.Encode.(
      value(`List([name |> string, fileType |> FileType.encode]))
    );
};
