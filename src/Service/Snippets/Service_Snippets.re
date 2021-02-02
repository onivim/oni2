open Oni_Core;
open Utility;

module Log = (val Log.withNamespace("Service_Snippets"));

module SnippetWithMetadata = {
  [@deriving show]
  type t = {
    snippet: string,
    prefix: string,
    description: string,
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
    };

    let decode =
      obj(({field, _}) =>
        {
          prefix: field.optional("prefix", string),
          body: field.required("body", snippetLines),
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
             };
           }),
         )
    );
};

module Cache = {
  let fileToPromise: Hashtbl.t(string, Lwt.t(list(SnippetWithMetadata.t))) =
    Hashtbl.create(16);

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

module Sub = {
  type snippetFileParams = {
    uniqueId: string,
    filePaths: list(Fp.t(Fp.absolute)),
  };
  module SnippetFileSubscription =
    Isolinear.Sub.Make({
      type nonrec msg = list(SnippetWithMetadata.t);
      type nonrec params = snippetFileParams;

      type state = unit;

      let name = "Service_Snippet.SnippetFileSubscription";
      let id = ({uniqueId, _}) => uniqueId;

      let init = (~params, ~dispatch) => {
        // Load all files
        // Coalesce all promises
        let promises =
          params.filePaths |> List.map(Fp.toString) |> List.map(Cache.get);

        let join = (a, b) => a @ b;
        let promise = LwtEx.some(~default=[], join, promises);

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

      let update = (~params as _, ~state, ~dispatch as _) => state;

      let dispose = (~params as _, ~state as _) => {
        ();
      };
    });
  let snippetFromFiles = (~uniqueId, ~filePaths, toMsg) => {
    SnippetFileSubscription.create({uniqueId, filePaths})
    |> Isolinear.Sub.map(toMsg);
  };
};
