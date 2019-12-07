/*
 * GrammarRepository.rei
 */

open Oni_Core;

module Ext = Oni_Extensions;

type t;

let empty: t;

let create:
  (~log: string => unit=?, Ext.LanguageInfo.t) => t;

let getGrammar: (~scope: string, t) => option(Textmate.Grammar.t);
