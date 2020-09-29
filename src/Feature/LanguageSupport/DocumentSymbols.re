open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;

type provider = {
  handle: int,
  selector: Exthost.DocumentSelector.t,
};

type symbol = {
  name: string,
  detail: string,
  kind: SymbolKind.t,
  range: CharacterRange.t,
  selectionRange: CharacterRange.t,
};

let rec extHostSymbolToTree: Exthost.DocumentSymbol.t => Tree.t(symbol, symbol) = 
    extSymbol => {
  let children = extSymbol.children
  |> List.map(extHostSymbolToTree);

  let symbol  = {
    name: extSymbol.name,
    detail: extSymbol.detail,
    kind: extSymbol.kind,
    range: extSymbol.range |> OneBasedRange.toRange,
    selectionRange: extSymbol.range |> OneBasedRange.toRange,
  };

  if (children.length == 0) {
    Tree.leaf(symbol)
  } else {
    Tree.node(~children, symbol)
  }
};

type model = {
  providers: list(provider),
  bufferToSymbols: IntMap.t(list(Tree.t(symbol, symbol)));
};

let initial = {providers: [], bufferToSymbols: IntMap.empty};

[@deriving show]
type msg =
  | DocumentSymbolsAvailable({
      bufferId: int,
      symbols: list(Exthost.DocumentSymbol.t),
    });

let update = (msg, model) => {
  model
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
  Isolinear.Sub.none
//  let toMsg = (highlights: list(Exthost.DocumentHighlight.t)) => {
//    let ranges =
//      highlights
//      |> List.map(({range, _}: Exthost.DocumentHighlight.t) => {
//           Exthost.OneBasedRange.toRange(range)
//         });
//
//    DocumentHighlighted({bufferId: Oni_Core.Buffer.getId(buffer), ranges});
//  };
//
//  model.providers
//  |> List.filter(({selector, _}) =>
//       selector |> Exthost.DocumentSelector.matchesBuffer(~buffer)
//     )
//  |> List.map(({handle, _}) => {
//       Service_Exthost.Sub.documentHighlights(
//         ~handle,
//         ~buffer,
//         ~position=location,
//         ~toMsg,
//         client,
//       )
//     })
//  |> Isolinear.Sub.batch;
};
