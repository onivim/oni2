open Oni_Core;
open Exthost;

[@deriving show]
type msg = unit;

[@deriving show]
type provider = {
  handle: int,
  selector: DocumentSelector.t,
  triggerCharacters: list(string),
  supportsResolveDetails: bool,
  extensionId: string,
};

[@deriving show]
type meet = {
  buffer: [@opaque] Oni_Core.Buffer.t,
  base: string,
  location: EditorCoreTypes.Location.t,
};

type model = {
  handleToMeet: IntMap.t(meet),
  providers: list(provider)
};

let initial = {
  handleToMeet: IntMap.empty,
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
    ...model,
    providers: [{handle, selector, triggerCharacters, supportsResolveDetails, extensionId}, ...model.providers]
};

let unregister = (~handle, model) => {
    ...model,
    providers: List.filter(prov => prov.handle != handle, model.providers)
};

let bufferUpdated = (
  ~buffer,
  ~activeCursor,
  ~syntaxScope,
  ~triggerKey,
  model
) => {
  
  let candidateProviders = model.providers
  |> List.filter(prov => Exthost.DocumentSelector.matchesBuffer(
    ~buffer, prov.selector
  ));

  let handleToMeet = List.fold_left((acc: IntMap.t(meet), curr: provider) => {
    let maybeMeet = CompletionMeet.fromBufferLocation(
      // TODO: triggerCharacters
      ~location=activeCursor,
      buffer
    );

    switch (maybeMeet) {
    | None => acc
    | Some({base, location, _}) =>
    let meet = {
      buffer,
      base,
      location
      
    };
    prerr_endline(
      Printf.sprintf("MEET %d: %s", curr.handle, show_meet(meet))
    );
      IntMap.add(curr.handle, meet, acc)
    }
  }, IntMap.empty, candidateProviders);
  
  {...model, handleToMeet}
}

let update = (msg, model) => {
  (model, Outmsg.Nothing)
};

let sub = (model) => Isolinear.Sub.none;
