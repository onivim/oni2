/*
  Theme.re
  */

open TextMateScopes;

type t = {
  selectors: list(Selector.t),
  trie: Trie.t(Selector.t),
};

let create = (~selectors: list(Selector.t), ()) => {
  open Selector;

  let trie =
    List.fold_left(
      (prev, curr) => {
        let {scopes, _} = curr;
        let scopeToAdd = List.rev(scopes) |> List.hd;

        let f = _ => Some(curr);

        Trie.update(scopeToAdd, f, prev);
      },
      Trie.empty,
      selectors,
    );

  {selectors, trie};
};

let match = (_theme: t, _scopes: Scopes.t) => {
  ResolvedStyle.default;
};
