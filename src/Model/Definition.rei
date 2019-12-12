/*
 * Definition.rei
 *
 * This module is responsible for recording and managing the 'definition' feature state
 */
open EditorCoreTypes;

type t;

let empty: t;

let getAt:
  (int, Location.t, t) => option(LanguageFeatures.DefinitionResult.t);

// [getRange(definition)] returns a Range.t with the associated source range of the definition
let getRange: t => option(Range.t);

let isAvailable: (int, Location.t, t) => bool;

let create: (int, Location.t, LanguageFeatures.DefinitionResult.t) => t;
