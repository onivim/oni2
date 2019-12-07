/*
 * TreesitterRepository.rei
 */

open Oni_Core;

module Ext = Oni_Extensions;

type t;

let empty: t;

let create:
  (~log: string => unit=?, Ext.LanguageInfo.t) => t;

let getScopeConverter: (~scope: string, t) => option(TreeSitterScopes.TextMateConverter.t);
