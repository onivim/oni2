/*
 * Definition.re
 *
 * This module is responsible for recording and managing the 'definition' feature state
 */
open EditorCoreTypes;
open Oni_Core;
open Utility;

module New = {
  type provider = {
    handle: int,
    selector: Exthost.DocumentSelector.t,
  };

  type model = {providers: list(provider)};

  [@deriving show]
  type msg = unit;

  let initial = {providers: []};

  let update = (_msg, model) => model;

  let register = (~handle, ~selector, model) => {
    ...model,
    providers: [{handle, selector}, ...model.providers],
  };

  let unregister = (~handle: int, model) => {
    ...model,
    providers: model.providers |> List.filter(prov => prov.handle != handle),
  };

  let getAt = (~bufferId, ~location, model) => {
    None;
  };
};

type definition = {
  bufferId: int,
  // The position the hover was requested
  requestPosition: Location.t,
  // result
  result: option(LanguageFeatures.DefinitionResult.t),
};

type t = option(definition);

let empty: t = None;

let getAt = (bufferId, position, definition: t) => {
  let getHover = definition =>
    if (bufferId === definition.bufferId
        && Location.equals(position, definition.requestPosition)) {
      definition.result;
    } else {
      None;
    };

  Option.bind(definition, getHover);
};

let getRange = (definition: t) => {
  definition
  |> OptionEx.flatMap(def => def.result)
  |> OptionEx.flatMap((res: LanguageFeatures.DefinitionResult.t) =>
       res.originRange
     );
};

let isAvailable = (bufferId, position, definition: t) => {
  getAt(bufferId, position, definition) != None;
};

let create = (bufferId, requestPosition, result) => {
  Some({bufferId, requestPosition, result: Some(result)});
};
