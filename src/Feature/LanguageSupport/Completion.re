open Oni_Core;
open Exthost;

type provider = {
  handle: int,
  selector: DocumentSelector.t,
  triggerCharacters: list(string),
  supportsResolveDetails: bool,
  extensionId: string,
};

type model = {
  providers: list(provider)
};

let initial = {
  providers: [],
};

let register = (
    ~handle,
    ~selector,
    ~triggerCharacters,
    ~supportsResolveDetails,
    ~extensionId,
    model
) => {
    providers: [{handle, selector, triggerCharacters, supportsResolveDetails, extensionId}, ...model.providers]

};

let unregister = (~handle, model) => {
    providers: List.filter(prov => prov.handle != handle, model.providers)
};


