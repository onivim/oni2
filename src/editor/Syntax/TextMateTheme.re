/*
 Theme.re
 */

open TextMateScopes;

module TokenStyle = TextMateScopes.TokenStyle;
module ResolvedStyle = TextMateScopes.ResolvedStyle;

type themeSelector = (string, TokenStyle.t);

type selectorWithParents = {
  style: TokenStyle.t,
  parents: list(Selector.t),
}

type t = {
  trie: Trie.t(selectorWithParents),
};

let _explodeSelectors = (s: string) => {
  s
  |> String.split_on_char(',')
  |> List.map(s => String.trim(s))
};

let create = (selectors: list(themeSelector)) => {
  let f = (v: themeSelector) => {
    let (s, style) = v;

    // Expand "foo, bar" -> ["foo", "bar"]
    let explodedSelectors = _explodeSelectors(s);

    List.map(scope => {
      Selector.create(
        ~style,
        ~scopes=Scopes.ofString(scope),
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
        open Selector;
        let {scopes, style} = curr;
        
        let revScopes = List.rev(scopes);
        switch (revScopes) {
        | [] => prev
        | [hd] => {

            // If this is the only node (no parent selector),
            // we just want to set the selector directly on the node
            let f = prev => switch(prev) {
            | None => Some({
              parents: [],
              style,
            })
            | Some(v) => Some({
              ...v,
              style,
            })
            }

            Trie.update(hd, f, prev);
        }
        | [hd, ...tail] => {

            let f = prev => switch(prev) {
            | None => Some({
              style: TokenStyle.default,
              parents: [{
                style,
                scopes: tail,
              }]
            })
            | Some(v) => Some({
              ...v,
              parents: [{ style, scopes: tail}, ...v.parents]
            })
            }

            Trie.update(hd, f, prev);
        }
        };
      },
      Trie.empty,
      selectors,
    );

  prerr_endline(
    Trie.show((i: selectorWithParents) => TokenStyle.show(i.style), trie),
  );
  let ret: t = {
    trie: trie,
  };
  
  ret;
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
        let (_, selector: option(selectorWithParents)) = curr;

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
