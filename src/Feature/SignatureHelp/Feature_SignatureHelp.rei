open Oni_Core;
open EditorCoreTypes;

[@deriving show({with_path: false})]
type provider = {
  handle: int,
  selector: list(Exthost.DocumentFilter.t),
  metadata: Exthost.SignatureHelp.ProviderMetadata.t,
};

type model;

let initial: model;

[@deriving show({with_path: false})]
type command =
  | Show;

[@deriving show({with_path: false})]
type msg =
  | Command(command)
  | ProviderRegistered(provider)
  | InfoReceived({
      signatures: list(Exthost.SignatureHelp.Signature.t),
      activeSignature: int,
      activeParameter: int,
      requestID: int,
    })
  | RequestFailed(string);

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | Error(string);

let update:
  (
    ~maybeBuffer: option(Buffer.t),
    ~maybeEditor: option(Feature_Editor.Editor.t),
    ~extHostClient: Exthost.Client.t,
    model,
    msg
  ) =>
  (model, outmsg);

module Contributions: {let commands: list(Command.t(msg));};
