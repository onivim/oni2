open Oni_Core;

module SnippetWithMetadata = {
  type t = {
    snippet: Snippet.t,
    prefix: string,
    description: string,
  };
};

// Decoder logic for snippet files
module Decode = {
  open Json.Decode;

  let snippetLines =
    list(string)
    |> and_then(lines => {
         let parseResult = lines |> String.concat("\n") |> Snippet.parse;

         switch (parseResult) {
         | Ok(snippet) => succeed(snippet)
         | Error(msg) => fail(msg)
         };
       });

  module PrefixAndBody = {
    type t = {
      prefix: option(string),
      body: Snippet.t,
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

  let _get = (filePath: string) => {
    switch (Hashtbl.find_opt(fileToPromise, filePath)) {
    | Some(promise) => promise
    | None =>
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
  let snippetFromFiles = (~filePaths as _, _toMsg) => Isolinear.Sub.none;
};
