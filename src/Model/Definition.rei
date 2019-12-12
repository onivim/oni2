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

let isAvailable: (int, Location.t, t) => bool;

let create: (int, Location.t, LanguageFeatures.DefinitionResult.t) => t;
