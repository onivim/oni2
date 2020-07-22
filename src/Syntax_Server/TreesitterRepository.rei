/*
 * TreesitterRepository.rei
 */

open Oni_Syntax;

type t;

let empty: t;

let create: (~log: string => unit=?, Exthost.GrammarInfo.t) => t;

let getScopeConverter:
  (~scope: string, t) => option(TreeSitterScopes.TextMateConverter.t);
