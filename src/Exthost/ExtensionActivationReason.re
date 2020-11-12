open Oni_Core;

type t = {
  startup: bool,
  extensionId: ExtensionId.t,
  activationEvent: string,
};

let create = (~startup: bool, ~extensionId: string, ~activationEvent) => {
  startup,
  extensionId,
  activationEvent,
};

let encode = ({startup, extensionId, activationEvent}) =>
  Json.Encode.(
    obj([
      ("startup", startup |> bool),
      ("extensionId", extensionId |> ExtensionId.encode),
      ("activationEvent", activationEvent |> string),
    ])
  );
