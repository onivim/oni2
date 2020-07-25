open Oni_Core;
module LaunchConfig = {
  [@deriving show]
  type t = {
    name: option(string),
    shellPath: option(string),
    shellArgs: list(string),
    cwd: option(Oni_Core.Uri.t),
    // TODO: Env
    //env:
    waitOnExit: bool,
    strictEnv: bool,
    hideFromUser: bool,
    isExtensionTerminal: bool,
  };

  module Decode = {
    open Json.Decode;

    let shellArgs =
      one_of([
        ("single string", string |> map(str => [str])),
        ("string list", list(string)),
      ]);

    let uri =
      one_of([
        ("string", string |> map(Uri.fromPath)),
        ("uri", Uri.decode),
      ]);
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          name: field.optional("name", string),
          shellPath: field.optional("shellPath", string),
          shellArgs: field.withDefault("shellArgs", [], Decode.shellArgs),
          cwd: field.optional("cwd", Decode.uri),
          waitOnExit: field.withDefault("waitOnExit", false, bool),
          strictEnv: field.withDefault("strictEnv", false, bool),
          hideFromUser: field.withDefault("hideFromUser", false, bool),
          isExtensionTerminal:
            field.withDefault("isExtensionTerminal", false, bool),
        }
      )
    );
};
