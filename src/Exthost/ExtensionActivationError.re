open Oni_Core;

type t =
  | Message(string)
  | MissingDependency(ExtensionId.t);

let toString =
  fun
  | Message(str) => str
  | MissingDependency(extensionId) =>
    Printf.sprintf(
      "Missing extension dependency missing: %s",
      ExtensionId.toString(extensionId),
    );

module Decode = {
  open Json.Decode;
  let message = string |> map(str => Message(str));

  let missingDependency =
    obj(({field, _}) => {field.required("dependency", ExtensionId.decode)})
    |> map(extensionId => MissingDependency(extensionId));

  let decode =
    one_of([
      ("message", message),
      ("missingDependency", missingDependency),
    ]);
};

let decode = Decode.decode;
