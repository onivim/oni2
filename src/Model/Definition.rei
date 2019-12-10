/*
 * Definition.rei
 *
 * This module is responsible for recording and managing the 'definition' feature state
 */
open Oni_Core;
module Ext = Oni_Extensions;

type t;

let empty: t;

let getAt: (int, Position.t, t) => option(Ext.LanguageFeatures.DefinitionResult.t);

let isAvailable: (int, Position.t, t) => bool;

let create: (int, Position.t, Ext.LanguageFeatures.DefinitionResult.t) => t;
