open Oni_Core;

type t = FpExp.t(Fp.absolute);

type scope =
  | Global
  | Language(string);

let global = (folder: FpExp.t(Fp.absolute)) => {
  FpExp.At.(folder / "global.code-snippets");
};

let language = (~fileType: string, folder: FpExp.t(Fp.absolute)) => {
  let fileName = fileType ++ ".json";
  FpExp.At.(folder / fileName);
};

let scope = (path: t) => {
  let splitPath =
    path |> FpExp.toString |> Filename.basename |> String.split_on_char('.');

  switch (splitPath) {
  | [] => None
  | [_] => None
  | [fileType, extension] when extension == "json" =>
    Some(Language(fileType))
  | [_file, extension] when extension == "code-snippets" => Some(Global)
  | _ => None
  };
};

let isGlobal = (path: t) => scope(path) == Some(Global);

let matches = (~fileType, snippetFile: t) => {
  switch (scope(snippetFile)) {
  | None => false
  | Some(Global) => true
  | Some(Language(languageId)) => languageId == fileType
  };
};

module Defaults = {
  // From:
  // https://github.com/microsoft/vscode/blob/848010a7257eec652d76dbbce579819e176b2bff/src/vs/workbench/parts/snippets/electron-browser/configureSnippets.ts#L141
  let global = {|
{
  // Place your global snippets here. Each snippet is defined under a snippet name and has a scope, prefix, body and
  // description. Add comma separated ids of the languages where the snippet is applicable in the scope field. If scope
  // is left empty or omitted, the snippet gets applied to all languages. The prefix is what is
  // used to trigger the snippet and the body will be expanded and inserted. Possible variables are:
  // $1, $2 for tab stops, $0 for the final cursor position, and ${1:label}, ${2:another} for placeholders.
  // Placeholders with the same ids are connected.
  // Example:
  // "Print to console": {
  //   "scope": "javascript,typescript",
  //   "prefix": "log",
  //   "body": [
  //     "console.log('$1');",
  //     "$2"
  //   ],
  //   "description": "Log output to console"
  // }
}|};

  let language = (~fileType) => {
    Printf.sprintf(
      {|
{
  // Place your snippets for %s here. Each snippet is defined under a snippet name and has a scope, prefix, body and
  // description. Add comma separated ids of the languages where the snippet is applicable in the scope field. If scope
  // is left empty or omitted, the snippet gets applied to all languages. The prefix is what is
  // used to trigger the snippet and the body will be expanded and inserted. Possible variables are:
  // $1, $2 for tab stops, $0 for the final cursor position, and ${1:label}, ${2:another} for placeholders.
  // Placeholders with the same ids are connected.
  // Example:
  // "Print to console": {
  //   "prefix": "log",
  //   "body": [
  //     "console.log('$1');",
  //     "$2"
  //   ],
  //   "description": "Log output to console"
  // }
}|},
      fileType,
    );
  };
};

let ensureCreated = (snippetFile: t) => {
  Lwt.catch(
    () => {
      snippetFile
      |> FpExp.toString
      |> Service_OS.Api.stat
      |> Lwt.map(_ => snippetFile)
    },
    _exn => {
      let maybeContents =
        scope(snippetFile)
        |> Option.map(
             fun
             | Global => Defaults.global
             | Language(fileType) => Defaults.language(~fileType),
           )
        |> Option.map(Bytes.of_string);

      maybeContents
      |> Option.map(contents => {
           Lwt.catch(
             () => {
               // File not created yet, let's create it
               Service_OS.Api.writeFile(
                 ~contents,
                 FpExp.toString(snippetFile),
               )
               |> Lwt.map(() => snippetFile)
             },
             exn => Lwt.fail_with(Printexc.to_string(exn)),
           )
         })
      |> Option.value(
           ~default=
             Lwt.fail_with(
               "Invalid snippet file path: " ++ FpExp.toString(snippetFile),
             ),
         );
    },
  );
};
