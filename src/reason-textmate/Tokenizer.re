/*
 Tokenizer.rei
 */

type t = {repository: GrammarRepository.t};

let create = (~repository: GrammarRepository.t, ()) => {
  let ret: t = {repository: repository};
  ret;
};

let tokenize =
    (
      ~lineNumber: int=0,
      ~scopeStack: option(ScopeStack.t)=None,
      ~scope,
      v: t,
      line,
    ) => {
  let repository = GrammarRepository.getGrammar(v.repository);

  switch (GrammarRepository.getGrammar(v.repository, scope)) {
  | Some(g) =>
    Grammar.tokenize(
      ~lineNumber,
      ~scopes=scopeStack,
      ~grammarRepository=repository,
      ~grammar=g,
      line,
    )
  | None => ([], ScopeStack.ofTopLevelScope([], scope))
  };
};
