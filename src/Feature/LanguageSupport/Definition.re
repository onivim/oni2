/*
 * Definition.re
 *
 * This module is responsible for recording and managing the 'definition' feature state
 */
open EditorCoreTypes;
open Oni_Core;
open Utility;

module Ext = Oni_Extensions;

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
  let getHover = definition => {
    if (bufferId === definition.bufferId
        && Location.equals(position, definition.requestPosition)) {
      definition.result;
    } else {
      None;
    };
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
