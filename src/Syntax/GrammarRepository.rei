/*
 * GrammarRepository.rei
 */

type t;

let empty: t;

let create: (~log: string => unit=?, Exthost.GrammarInfo.t) => t;

let getGrammar: (~scope: string, t) => option(Textmate.Grammar.t);
