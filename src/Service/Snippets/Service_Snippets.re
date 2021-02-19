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
    filePath: [@opaque] FpExp.t(FpExp.absolute),
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
            Log.infof(m => m("Reading json for: %s", filePath));
            let json = bytes |> Bytes.to_string |> Yojson.Safe.from_string;

            let parseResult = Json.Decode.decode_value(Decode.decode, json);

            switch (parseResult) {
            | Ok(snippets) =>
              Log.infof(m =>
                m(
                  "Read %d snippets from %s",
                  List.length(snippets),
                  filePath,
                )
              );
              Lwt.return(snippets);
            | Error(msg) =>
              let msgStr = Json.Decode.string_of_error(msg);
              Log.errorf(m =>
                m("Parsing snippet file %s failed with: %s", filePath, msgStr)
              );
              Lwt.fail_with(msgStr);
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

  let readSnippetFilesFromFolder = folder => {
    folder
    |> FpExp.toString
    |> Service_OS.Api.readdir
    |> Lwt.map(dirents => {
         dirents
         |> List.map((dirent: Luv.File.Dirent.t) =>
              FpExp.At.(folder / dirent.name)
            )
         |> List.filter(file => SnippetFile.scope(file) != None)
       });
  };

  let loadSnippetsFromFolder = (~fileType, folder) => {
    readSnippetFilesFromFolder(folder)
    |> LwtEx.flatMap(snippetFiles => {
         snippetFiles
         |> List.filter(SnippetFile.matches(~fileType))
         |> List.map((dir: FpExp.t(FpExp.absolute)) => {
              let str = FpExp.toString(dir);
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
    let promises =
      filePaths |> List.map(FpExp.toString) |> List.map(Cache.get);

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
      let createPromise = SnippetFile.ensureCreated(filePath);
      Lwt.on_any(
        createPromise,
        filePath => {dispatch(toMsg(Ok(filePath)))},
        exn => {dispatch(toMsg(Error(Printexc.to_string(exn))))},
      );
    });
  };
  let clearCachedSnippets = (~filePath) => {
    Isolinear.Effect.create(~name="Service_Snippets.clearCachedSnippets", () => {
      Log.tracef(m =>
        m("Clearing snippet cache for file: %s", filePath |> FpExp.toString)
      );
      Cache.clear(FpExp.toString(filePath));
    });
  };
  let snippetFromFiles = (~fileType, ~filePaths, toMsg) =>
    Isolinear.Effect.createWithDispatch(
      ~name="Service_Snippets.Effect.snippetFromFiles", dispatch => {
      Internal.loadSnippetsFromFiles(~filePaths, ~fileType, snippets =>
        dispatch(toMsg(snippets))
      )
    });

  let getUserSnippetFiles = (~languageInfo, toMsg) => {
    // TODO
    Isolinear.Effect.createWithDispatch(
      ~name="Service_Snippets.Effect.getUserSnippetFiles", dispatch => {
      switch (Filesystem.getSnippetsFolder()) {
      // TODO: Error logging
      | Error(_) => dispatch(toMsg([]))
      | Ok(userSnippetsFolder) =>
        let userSnippetsPromise =
          Internal.readSnippetFilesFromFolder(userSnippetsFolder);

        let promise =
          userSnippetsPromise
          |> Lwt.map(userSnippets => {
               let alreadyCreatedLanguages =
                 userSnippets
                 |> List.fold_left(
                      (set, file) => {
                        switch (SnippetFile.scope(file)) {
                        | Some(Language(languageId)) =>
                          StringSet.add(languageId, set)
                        | Some(Global)
                        | None => set
                        }
                      },
                      StringSet.empty,
                    );

               // Get a list of candidate snippets
               let candidateSnippets =
                 Exthost.LanguageInfo.languages(languageInfo)
                 |> List.map(fileType =>
                      SnippetFile.language(~fileType, userSnippetsFolder)
                    )
                 |> List.filter(candidateFile => {
                      switch (SnippetFile.scope(candidateFile)) {
                      | Some(Global) => true
                      | Some(Language(language)) =>
                        !StringSet.mem(language, alreadyCreatedLanguages)
                      | None => false
                      }
                    });

               // Do we already have a global snippet?
               let hasGlobalSnippet =
                 userSnippets |> List.exists(SnippetFile.isGlobal);

               let toMetadata = (~isCreated, snippetFile) => {
                 let language =
                   switch (SnippetFile.scope(snippetFile)) {
                   | Some(Language(language)) => Some(language)
                   | Some(Global)
                   | None => None
                   };
                 SnippetFileMetadata.{
                   isCreated,
                   filePath: snippetFile,
                   language,
                 };
               };

               let existingSnippets =
                 userSnippets |> List.map(toMetadata(~isCreated=true));

               let newSnippets =
                 (
                   hasGlobalSnippet
                     ? candidateSnippets
                     : [
                       SnippetFile.global(userSnippetsFolder),
                       ...candidateSnippets,
                     ]
                 )
                 |> List.map(toMetadata(~isCreated=false));
               existingSnippets @ newSnippets;
             });

        Lwt.on_success(promise, snippetFiles =>
          dispatch(toMsg(snippetFiles))
        );
        Lwt.on_failure(promise, _exn => dispatch(toMsg([])));
      }
    });
  };
};

module Sub = {
  type snippetFileParams = {
    uniqueId: string,
    filePaths: list(FpExp.t(FpExp.absolute)),
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
