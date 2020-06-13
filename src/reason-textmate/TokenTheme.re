/*
 TokenTheme.re
 */

open ThemeScopes;

module TokenStyle = ThemeScopes.TokenStyle;
module ResolvedStyle = ThemeScopes.ResolvedStyle;

type themeSelector = (string, TokenStyle.t);

type selectorWithParents = {
  // Style is optional, because it's possible for a Trie node to _only_
  // have parent selectors. In that case, if no parent selectors match,
  // we want to continue evaluating styles.
  style: option(TokenStyle.t),
  parents: list(Selector.t),
};

type t = {
  defaultBackground: string,
  defaultForeground: string,
  trie: Trie.t(selectorWithParents),
  selectors: list(themeSelector),
};

/* Helper to split the selectors on ',' for groups */
let _explodeSelectors = (s: string) => {
  s |> String.split_on_char(',') |> List.map(s => String.trim(s));
};

let create =
    (~defaultBackground, ~defaultForeground, selectors: list(themeSelector)) => {
  let originalSelectors = selectors;
  let f = (v: themeSelector) => {
    let (s, style) = v;

    // Expand "foo, bar" -> ["foo", "bar"]
    let explodedSelectors = _explodeSelectors(s);

    List.map(
      scope => Selector.create(~style, ~scopes=Scopes.ofString(scope), ()),
      explodedSelectors,
    );
  };
  let selectors = selectors |> List.map(f) |> List.flatten;

  let trie =
    List.fold_left(
      (prev, curr) => {
        open Selector;
        let {scopes, style} = curr;

        let revScopes = List.rev(scopes);
        switch (revScopes) {
        | [] => prev
        | [hd] =>
          // If this is the only node (no parent selector),
          // we just want to set the selector directly on the node
          let f = prev =>
            switch (prev) {
            | None => Some({parents: [], style: Some(style)})
            | Some(v) => Some({...v, style: Some(style)})
            };

          Trie.update(hd, f, prev);
        | [hd, ...tail] =>
          let f = prev =>
            switch (prev) {
            | None =>
              Some({
                // This is the 'parent selector' case - we're adding a parent selector to a Trie node,
                // but it has no non-parent style of its own.
                style: None,
                parents: [{style, scopes: tail}],
              })
            | Some(v) =>
              Some({...v, parents: [{style, scopes: tail}, ...v.parents]})
            };

          Trie.update(hd, f, prev);
        };
      },
      Trie.empty,
      selectors,
    );

  let ret: t = {
    defaultBackground,
    defaultForeground,
    trie,
    selectors: originalSelectors,
  };

  ret;
};

let union = (~defaultBackground, ~defaultForeground, a: t, b: t) => {
  let allSelectors = List.flatten([a.selectors, b.selectors]);
  create(~defaultBackground, ~defaultForeground, allSelectors);
};

let of_yojson = (~defaultBackground, ~defaultForeground, json: Yojson.Safe.t) => {
  let parseSettings: Yojson.Safe.t => TokenStyle.t =
    json => {
      let str = v =>
        switch (v) {
        | `String(s) => Some(s)
        | _ => None
        };

      let boo = v =>
        switch (v) {
        | `Bool(s) => Some(s)
        | _ => None
        };

      print_endline(json |> Yojson.Safe.to_string);

      let fontStyle = fs =>
        Yojson.Safe.Util.member("fontStyle", json)
        |> str
        |> Option.map(s =>
             {
               print_endline(s);
               s;
             }
             == fs
           );

      TokenStyle.create(
        ~foreground=str(Yojson.Safe.Util.member("foreground", json)),
        ~background=str(Yojson.Safe.Util.member("background", json)),
        ~bold=fontStyle("bold"),
        ~italic=fontStyle("italic"),
        (),
      );
    };

  let parseStringList = (arr: list(Yojson.Safe.t)) => {
    List.fold_left(
      (prev, curr) =>
        switch (curr) {
        | `String(v) => [v, ...prev]
        | _ => prev
        },
      [],
      arr,
    );
  };

  let parseSelector = (selector: Yojson.Safe.t) => {
    switch (selector) {
    | `Assoc(_) =>
      let scope = Yojson.Safe.Util.member("scope", selector);
      let settings = Yojson.Safe.Util.member("settings", selector);

      switch (scope, settings) {
      | (`List(v), `Assoc(_)) =>
        let tokenStyle = parseSettings(settings);
        let selector = parseStringList(v) |> String.concat(",");
        [(selector, tokenStyle)];
      | (`String(v), `Assoc(_)) =>
        let tokenStyle = parseSettings(settings);
        let selector = v;
        [(selector, tokenStyle)];
      | _ => []
      };
    | _ => []
    };
  };

  let selectors =
    switch (json) {
    | `List(elems) => List.map(parseSelector, elems) |> List.flatten
    | _ => []
    };

  create(~defaultBackground, ~defaultForeground, selectors);
};

module PlistDecoder = {
  open Plist;

  let settings =
    dict(prop =>
      TokenStyle.{
        foreground: prop.optional("foreground", string),
        background: prop.optional("background", string),
        bold: prop.optional("bold", bool),
        italic: prop.optional("italic", bool),
      }
    );

  let selector =
    dict(prop =>
      (prop.required("scope", string), prop.required("settings", settings))
    );

  let tokenTheme = (~defaultBackground, ~defaultForeground) =>
    array(selector)
    |> map(selectors =>
         create(~defaultBackground, ~defaultForeground, selectors)
       );
};

let of_plist = PlistDecoder.tokenTheme;

let empty = create(~defaultBackground="#000", ~defaultForeground="#fff", []);

let show = (v: t) => {
  Trie.show(
    (i: selectorWithParents) => {
      switch (i.style) {
      | Some(v) => TokenStyle.show(v)
      | None => "(None)"
      }
    },
    v.trie,
  );
};

let _applyStyle: (TokenStyle.t, TokenStyle.t) => TokenStyle.t =
  (prev: TokenStyle.t, style: TokenStyle.t) => {
    let foreground =
      switch (prev.foreground, style.foreground) {
      | (Some(v), _) => Some(v)
      | (_, Some(v)) => Some(v)
      | _ => None
      };

    let background =
      switch (prev.background, style.background) {
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

    {background, foreground, bold, italic};
  };

let match = (theme: t, scopes: string) => {
  let scopes = Scopes.ofString(scopes) |> List.rev;
  let default =
    ResolvedStyle.default(
      ~foreground=theme.defaultForeground,
      ~background=theme.defaultBackground,
      (),
    );

  let rec f = scopes => {
    switch (scopes) {
    | [] => default
    | [scope, ...scopeParents] =>
      // Get the matching path from the Trie
      let p = Trie.matches(theme.trie, scope);

      switch (p) {
      // If there were no matches... try the next scope up.
      | [] => f(scopeParents)
      // Got matches - we'll apply them in sequence
      | _ =>
        let result =
          List.fold_left(
            (prev: option(TokenStyle.t), curr) => {
              let (_name, selector: option(selectorWithParents)) = curr;

              switch (selector) {
              // No selector at this node. This can happen when a node is on the
              // path to a node with a style. Nothing to do here; continue on.
              | None => prev
              // We have a selector at this node. Let's check it out.
              | Some({style, parents}) =>
                let prevStyle =
                  switch (prev) {
                  | None => TokenStyle.default
                  | Some(v) => v
                  };

                // Get the list of matching parent selectors to apply
                let parentsScopesToApply =
                  parents
                  |> List.filter(selector =>
                       Selector.matches(selector, scopeParents)
                     );

                switch (parentsScopesToApply, style) {
                // Case 1: No parent selectors match AND there is no style. We should continue on.
                | ([], None) => None
                // Case 2: No parent selectors match, but there is a style at the Node. We should apply the style.
                | ([], Some(style)) => Some(_applyStyle(prevStyle, style))
                // Case 3: We have parent selectors that match, and may or may not have a style at the node.
                // Apply the parent styles, and the node style, if applicable.
                | (_, style) =>
                  let newStyle =
                    switch (style) {
                    | Some(v) => _applyStyle(prevStyle, v)
                    | None => TokenStyle.default
                    };
                  // Apply any parent selectors that match...
                  // we should be sorting this by score!
                  Some(
                    List.fold_left(
                      (prev, curr: Selector.t) => {
                        open Selector;
                        let {style, _} = curr;
                        // Reversing the order because the parent style
                        // should take precedence over previous style
                        _applyStyle(style, prev);
                      },
                      newStyle,
                      parentsScopesToApply,
                    ),
                  );
                };
              };
            },
            None,
            p,
          );

        switch (result) {
        | None => f(scopeParents)
        | Some(result) =>
          let foreground =
            switch (result.foreground) {
            | Some(v) => v
            | None => default.foreground
            };

          let bold =
            switch (result.bold) {
            | Some(v) => v
            | None => default.bold
            };

          let italic =
            switch (result.italic) {
            | Some(v) => v
            | None => default.italic
            };

          let background =
            switch (result.background) {
            | Some(v) => v
            | None => default.background
            };

          {background, foreground, bold, italic};
        };
      };
    };
  };
  f(scopes);
};
