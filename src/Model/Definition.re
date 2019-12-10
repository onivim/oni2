/*
 * Definition.re
 *
 * This module is responsible for recording and managing the 'definition' feature state
 */
open Oni_Core;
module Ext = Oni_Extensions;

type definition = {
  bufferId: int,
  // The position the hover was requested
  requestPosition: Position.t,

  // result
  result: option(Ext.LanguageFeatures.DefinitionResult.t),
};

type t = option(definition);

let empty: t = None;

let getAt = (bufferId, position, definition: t) => {
  let getHover = (definition) => {
    if (bufferId === definition.bufferId && Position.equals(position, definition.requestPosition)) {
      definition.result
    } else {
      None
    }
  };

  definition
  |> Utility.Option.bind(getHover);
};

let clear = () => empty;

let set = (bufferId, requestPosition, result) => {
  Some({
  bufferId,
  requestPosition,
  result: Some(result),
  })
};
