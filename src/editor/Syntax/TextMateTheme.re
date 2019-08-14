/*
 Theme.re
 */

open TextMateScopes;

module TokenStyle = TextMateScopes.TokenStyle;
module ResolvedStyle = TextMateScopes.ResolvedStyle;

type themeSelector = (string, TokenStyle.t);

type t = {
  selectors: list(Selector.t),
  trie: Trie.t(Selector.t),
};

let _explodeSelectors = (s: string) => {
  s
  |> String.split_on_char(',')
  |> List.map(s => String.trim(s))
};

let create = (selectors: list(themeSelector)) => {
  open Selector;

  let f = (v: themeSelector) => {
    let (s, style) = v;

    // Expand "foo, bar" -> ["foo", "bar"]
    let explodedSelectors = _explodeSelectors(s);

    List.map(scope => {
      Selector.create(
        ~style,
        ~scopes=[Scope.ofString(scope)],
        (),
      )
    }, explodedSelectors);
  };
  let selectors = selectors
    |> List.map(f)
    |> List.flatten;

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

  prerr_endline(
    Trie.show((i: Selector.t) => TokenStyle.show(i.style), trie),
  );
  {selectors, trie};
};

/* [match] returns the resolved style information,
   given the [scopes]. The [scopes] should include
   the full ancestor list, separated by spaces, for example:
   "text.html.basic source.php string.quoted.double.php"
*/
let match = (theme: t, scopes: string) => {
  let scopes = [Scope.ofString(scopes)];
  let scope = List.hd(scopes);
  let p = Trie.matches(theme.trie, scope);

  let result =
    List.fold_left(
      (prev: TokenStyle.t, curr) => {
        let (_, selector: option(Selector.t)) = curr;

        switch (selector) {
        | None => prev
        | Some({style, _}) =>
          let foreground =
            switch (prev.foreground, style.foreground) {
            | (Some(v), _) => Some(v)
            | (_, Some(v)) => Some(v)
            | _ => None
            };

          let bold =
            switch (prev.bold, style.bold) {
            | (Some(v), _) => Some(v)
            | (_, Some(v)) => Some(v)
            | _ => None
            };

          let italic =
            switch (prev.italic, style.italic) {
            | (Some(v), _) => Some(v)
            | (_, Some(v)) => Some(v)
            | _ => None
            };

          {...prev, foreground, bold, italic};
        };
      },
      TokenStyle.default,
      p,
    );

  let foreground =
    switch (result.foreground) {
    | Some(v) => v
    | None => ResolvedStyle.default.foreground
    };

  let bold =
    switch (result.bold) {
    | Some(v) => v
    | None => ResolvedStyle.default.bold
    };

  let italic =
    switch (result.italic) {
    | Some(v) => v
    | None => ResolvedStyle.default.italic
    };

  {...ResolvedStyle.default, foreground, bold, italic};
};
