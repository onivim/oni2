/*
 * Definition.rei
 *
 * This module is responsible for recording and managing the 'definition' feature state
 */

module New: {
  type model;

  let initial: model;

  [@deriving show]
  type msg;

  let update: (msg, model) => model;

  let register:
    (~handle: int, ~selector: Exthost.DocumentSelector.t, model) => model;
  let unregister: (~handle: int, model) => model;
};

open EditorCoreTypes;

type t;

let empty: t;

let getAt:
  (int, Location.t, t) => option(LanguageFeatures.DefinitionResult.t);

// [getRange(definition)] returns a Range.t with the associated source range of the definitiocompletionsn
let getRange: t => option(Range.t);

let isAvailable: (int, Location.t, t) => bool;

let create: (int, Location.t, LanguageFeatures.DefinitionResult.t) => t;
