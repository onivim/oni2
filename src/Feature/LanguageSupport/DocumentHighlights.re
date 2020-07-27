open EditorCoreTypes;
open Oni_Core;

type provider = {
  handle: int,
  selector: Exthost.DocumentSelector.t,
};

type model = {
  providers: list(provider),
  bufferToHighlights: IntMap.t(list(Range.t)),
};

let initial = {providers: [], bufferToHighlights: IntMap.empty};

[@deriving show]
type msg =
  | DocumentHighlighted({
      bufferId: int,
      ranges: list(Range.t),
    });
// TODO: kind?

let update = (msg, model) => {
  switch (msg) {
  | DocumentHighlighted({bufferId, ranges}) =>
    let bufferToHighlights =
      model.bufferToHighlights |> IntMap.add(bufferId, ranges);
    {...model, bufferToHighlights};
  };
};

let register = (~handle: int, ~selector, model) => {
  ...model,
  providers: [{handle, selector}, ...model.providers],
};

let unregister = (~handle: int, model) => {
  ...model,
  providers: model.providers |> List.filter(prov => prov.handle != handle),
};

let sub = (~buffer, ~location, ~client, model) => {
  let toMsg = (highlights: list(Exthost.DocumentHighlight.t)) => {
    let ranges =
      highlights
      |> List.map(({range, _}: Exthost.DocumentHighlight.t) => {
           Exthost.OneBasedRange.toRange(range)
         });

    DocumentHighlighted({bufferId: Oni_Core.Buffer.getId(buffer), ranges});
  };

  model.providers
  |> List.filter(({selector, _}) =>
       selector |> Exthost.DocumentSelector.matchesBuffer(~buffer)
     )
  |> List.map(({handle, _}) => {
       Service_Exthost.Sub.documentHighlights(
         ~handle,
         ~buffer,
         ~position=location,
         ~toMsg,
         client,
       )
     })
  |> Isolinear.Sub.batch;
};
