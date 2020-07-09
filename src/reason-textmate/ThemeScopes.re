/*

 ThemeScopes.re
 */

/*
   [scope] is a list representation of TextMate scopes.
   For example, "source.js" would be represented as ["source", "js"]
 */

module Scope = {
  type t = list(string);

  let ofString = s => String.split_on_char('.', s);

  let rec matches = (selector: t, v: t) => {
    switch (selector, v) {
    | ([], _) => true
    | ([selectorHd, ...selectorTl], [scopeHd, ...scopeTl]) =>
      if (String.equal(selectorHd, scopeHd)) {
        matches(selectorTl, scopeTl);
      } else {
        false;
      }
    | (_, []) => false
    };
  };
};

module Scopes = {
  /*
     In general, tokens have multiple scopes. For example,
     a token in a function might have the follow scopes:
     - "source.js"
     - "meta.definition.function.js"
     - "entity.name.function.js"

     The [t] type models this, as each bullet is a [scope].
   */
  type t = list(Scope.t);

  let ofString = s =>
    s
    |> String.split_on_char(' ')
    |> List.map(v => Scope.ofString(String.trim(v)));

  let rec matches = (selector: t, v: t) => {
    switch (selector, v) {
    | ([], _) => true
    | ([selectorHd, ...selectorTl], [scopeHd, ...scopeTl]) =>
      if (Scope.matches(selectorHd, scopeHd)) {
        matches(selectorTl, scopeTl);
      } else {
        matches(selector, scopeTl);
      }
    | (_, []) => false
    };
  };
};

module TokenStyle = {
  [@deriving show({with_path: false})]
  type t = {
    foreground: option(string),
    background: option(string),
    bold: option(bool),
    italic: option(bool),
  };

  let show = (v: t) => {
    switch (v.foreground) {
    | None => "Foreground: None"
    | Some(_) => "Foreground: Some"
    };
  };

  let create =
      (
        ~foreground: option(string)=None,
        ~background: option(string)=None,
        ~bold: option(bool)=None,
        ~italic: option(bool)=None,
        (),
      ) => {
    foreground,
    background,
    bold,
    italic,
  };

  let default = {
    foreground: None,
    background: None,
    bold: None,
    italic: None,
  };
};

module ResolvedStyle = {
  type t = {
    foreground: string,
    background: string,
    bold: bool,
    italic: bool,
  };

  let default = (~foreground, ~background, ()) => {
    foreground,
    background,
    bold: false,
    italic: false,
  };
};

module Selector = {
  type t = {
    scopes: Scopes.t,
    style: TokenStyle.t,
  };

  let create = (~style=TokenStyle.default, ~scopes, ()) => {scopes, style};

  let matches = (selector: t, scopes: Scopes.t) =>
    Scopes.matches(selector.scopes, scopes);
};
