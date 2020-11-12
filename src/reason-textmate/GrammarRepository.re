/*
 GrammarRepository.re
 */

type grammarRepository = string => option(Grammar.t);
type t = grammarRepository;

let getGrammar = (repository, scope) => repository(scope);

let ofGrammar = (scope, grammar, s) =>
  switch (s) {
  | v when v == scope => Some(grammar)
  | _ => None
  };

let ofFilePath = (scope: string, path: string) => {
  switch (Grammar.Json.of_file(path)) {
  | Ok(g) => ofGrammar(scope, g)
  | Error(_) => (_ => None)
  };
};

let create = v => v;
