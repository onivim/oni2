open Oni_Core;
open Utility;

module Log = (val Log.withNamespace("Service_Snippets"));

module SnippetWithMetadata = {
  [@deriving show]
  type t = {
    snippet: string,
    prefix: string,
    description: string,
    scopes: list(string),
  };

  let matchesFileType = (~fileType, {scopes, _}) => {
    // If no scope is defined, or empty, it's assumed to be global
    scopes == []
    || scopes == [""]
    || scopes
    |> List.exists(String.equal(fileType));
  };
};

module SnippetFileMetadata = {
  [@deriving show]
  type t = {
    language: option(string),
    filePath: [@opaque] Fp.t(Fp.absolute),
    isCreated: bool,
  };
};

// Decoder logic for snippet files
module Decode = {
  open Json.Decode;

  let snippetLines =
    list(string) |> map(lines => {lines |> String.concat("\n")});

  module PrefixAndBody = {
    type t = {
      prefix: option(string),
      body: string,
      scopes: list(string),
    };

    let decode =
      obj(({field, _}) =>
        {
          prefix: field.optional("prefix", string),
          body: field.required("body", snippetLines),
          scopes:
            field.withDefault(
              "scopes",
              [],
              string
              |> map(str =>
                   str |> String.split_on_char(',') |> List.map(String.trim)
                 ),
            ),
        }
      );
  };

  let decode =
    Json.Decode.(
      key_value_pairs(PrefixAndBody.decode)
      |> map(
           List.map(((description, prefixAndBody: PrefixAndBody.t)) => {
             let prefix =
               prefixAndBody.prefix |> Option.value(~default=description);

             SnippetWithMetadata.{
               prefix,
               description,
               snippet: prefixAndBody.body,
               scopes: prefixAndBody.scopes,
             };
           }),
         )
    );
};

module Cache = {
  let fileToPromise: Hashtbl.t(string, Lwt.t(list(SnippetWithMetadata.t))) =
    Hashtbl.create(16);

  let clear = (filePath: string) => {
    Hashtbl.remove(fileToPromise, filePath);
  };

  let get = (filePath: string) => {
    switch (Hashtbl.find_opt(fileToPromise, filePath)) {
    | Some(promise) =>
      Log.tracef(m => m("Cache.get - using cached result for %s", filePath));
      promise;
    | None =>
      Log.tracef(m =>
        m("Cache.get - no cached result for %s, loading...", filePath)
      );
      let promise =
        Lwt.bind(
          Service_OS.Api.readFile(filePath),
          bytes => {
            let json = bytes |> Bytes.to_string |> Yojson.Safe.from_string;

            let parseResult = Json.Decode.decode_value(Decode.decode, json);

            switch (parseResult) {
            | Ok(snippets) => Lwt.return(snippets)
            | Error(msg) => Lwt.fail_with(Json.Decode.string_of_error(msg))
            };
          },
        );
      Hashtbl.add(fileToPromise, filePath, promise);
      promise;
    };
  };
};

module Internal = {
  let join = (a, b) => a @ b;

  let loadSnippetsFromFolder = (~fileType, folder) => {
    folder
    |> Fp.toString
    |> Service_OS.Api.readdir
    |> LwtEx.flatMap(dirents => {
         dirents
         |> List.map((dirent: Luv.File.Dirent.t) =>
              Fp.At.(folder / dirent.name)
            )
         |> List.filter(SnippetFile.matches(~fileType))
         |> List.map((dir: Fp.t(Fp.absolute)) => {
              let str = Fp.toString(dir);
              Cache.get(str)
              |> Lwt.map(
                   List.filter(
                     SnippetWithMetadata.matchesFileType(~fileType),
                   ),
                 );
            })
         |> LwtEx.some(~default=[], join)
       });
  };

  let loadSnippetsFromFiles = (~filePaths, ~fileType, dispatch) => {
    // Load all files
    // Coalesce all promises
    let promises = filePaths |> List.map(Fp.toString) |> List.map(Cache.get);

    let userPromise: Lwt.t(list(SnippetWithMetadata.t)) =
      Filesystem.getSnippetsFolder()
      |> Result.map(loadSnippetsFromFolder(~fileType))
      |> ResultEx.value(~default=Lwt.return([]));

    let promise = LwtEx.some(~default=[], join, [userPromise, ...promises]);

    Lwt.on_success(
      promise,
      snippets => {
        Log.infof(m => m("Loaded %d snippets", List.length(snippets)));
        dispatch(snippets);
      },
    );

    Lwt.on_failure(
      promise,
      exn => {
        Log.errorf(m =>
          m("Error loading snippets: %s", Printexc.to_string(exn))
        );
        dispatch([]);
      },
    );
  };
};

module Effect = {
  let createSnippetFile = (~filePath, toMsg) => {
    Isolinear.Effect.createWithDispatch(
      ~name="Service_Snippets.createSnippetFile", dispatch => {
      let filePathString = Fp.toString(filePath);
      let statPromise = Service_OS.Api.stat(filePathString);
      Lwt.on_any(
        statPromise,
        (_stat: Luv.File.Stat.t) => {dispatch(toMsg(Ok(filePath)))},
        _exn => {
          // File not created yet, let's create it
          let contents = "Hello, world" |> Bytes.of_string;
          let writePromise =
            Service_OS.Api.writeFile(~contents, filePathString);
          Lwt.on_any(
            writePromise,
            () => {dispatch(toMsg(Ok(filePath)))},
            exn => {dispatch(toMsg(Error(Printexc.to_string(exn))))},
          );
        },
      );
    });
  };
  let clearCachedSnippets = (~filePath) => {
    Isolinear.Effect.create(~name="Service_Snippets.clearCachedSnippets", () => {
      Cache.clear(Fp.toString(filePath))
    });
  };
  let snippetFromFiles = (~fileType, ~filePaths, toMsg) =>
    Isolinear.Effect.createWithDispatch(
      ~name="Service_Snippets.Effect.snippetFromFiles", dispatch => {
      Internal.loadSnippetsFromFiles(~filePaths, ~fileType, snippets =>
        dispatch(toMsg(snippets))
      )
    });

  let getUserSnippetFiles = (~languageInfo as _, _) => {
    // TODO
    Isolinear.Effect.none;
  };
};

module Sub = {
  type snippetFileParams = {
    uniqueId: string,
    filePaths: list(Fp.t(Fp.absolute)),
    fileType: string,
  };
  module SnippetFileSubscription =
    Isolinear.Sub.Make({
      type nonrec msg = list(SnippetWithMetadata.t);
      type nonrec params = snippetFileParams;

      type state = unit;

      let name = "Service_Snippet.SnippetFileSubscription";
      let id = ({uniqueId, _}) => uniqueId;

      let init = (~params, ~dispatch) => {
        Internal.loadSnippetsFromFiles(
          ~fileType=params.fileType,
          ~filePaths=params.filePaths,
          dispatch,
        );
      };

      let update = (~params as _, ~state, ~dispatch as _) => state;

      let dispose = (~params as _, ~state as _) => {
        ();
      };
    });
  let snippetFromFiles = (~uniqueId, ~fileType, ~filePaths, toMsg) => {
    SnippetFileSubscription.create({uniqueId, filePaths, fileType})
    |> Isolinear.Sub.map(toMsg);
  };
};
