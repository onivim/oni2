open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;

type provider = {
  handle: int,
  selector: Exthost.DocumentSelector.t,
};

type model = {
  providers: list(provider),
  // buffer Id -> lines -> ranges
  bufferToHighlights: IntMap.t(IntMap.t(list(CharacterRange.t))),
};

let initial = {providers: [], bufferToHighlights: IntMap.empty};

[@deriving show]
type msg =
  | DocumentHighlighted({
      bufferId: int,
      ranges: list(CharacterRange.t),
    });
// TODO: kind?

let update = (msg, model) => {
  switch (msg) {
  | DocumentHighlighted({bufferId, ranges}) =>
    let lineMap = ranges |> Utility.RangeEx.toCharacterLineMap;
    let bufferToHighlights =
      model.bufferToHighlights |> IntMap.add(bufferId, lineMap);
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

let getByLine = (~bufferId, ~line, model) => {
  model.bufferToHighlights
  |> IntMap.find_opt(bufferId)
  |> OptionEx.flatMap(IntMap.find_opt(line))
  |> Option.value(~default=[]);
};

let getLinesWithHighlight = (~bufferId, model) => {
  model.bufferToHighlights
  |> IntMap.find_opt(bufferId)
  |> Option.map(lineMap => IntMap.bindings(lineMap) |> List.map(fst))
  |> Option.value(~default=[]);
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
