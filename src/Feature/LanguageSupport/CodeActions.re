open Oni_Core;
open Exthost;

type provider = {
  handle: int,
  selector: DocumentSelector.t,
  metadata: CodeAction.ProviderMetadata.t,
  displayName: string,
  supportsResolve: bool,
};

type model = {providers: list(provider)};

[@deriving show]
type msg = unit;

let initial = {providers: []};

let register =
    (~handle, ~selector, ~metadata, ~displayName, ~supportsResolve, model) => {
  {
    providers: [
      {handle, selector, metadata, displayName, supportsResolve},
      ...model.providers,
    ],
  };
};

let unregister = (~handle, model) => {
  providers:
    model.providers |> List.filter(provider => {provider.handle != handle}),
};

let update = (msg, model) => (model, Outmsg.Nothing);

let sub =
    (
      ~buffer,
      ~topVisibleBufferLine,
      ~bottomVisibleBufferLine,
      ~client,
      codeActions,
    ) => {
  Isolinear.Sub.none;
};
