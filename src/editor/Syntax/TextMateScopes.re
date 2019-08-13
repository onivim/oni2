/*
  TextMateScopes
  */

/*
     [scope] is a list representation of TextMate scopes.
     For example, "source.js" would be represented as ["source", "js"]
   */

module Scope = {
  type t = list(string);

  let ofString = s => String.split_on_char('.', s);
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
};

module TokenStyle = {
  type t = {
    foreground: option(int),
    background: option(int),
    bold: option(bool),
    italic: option(bool),
  };

  let create =
      (
        ~foreground: option(int)=?,
        ~background: option(int)=?,
        ~bold: option(bool)=?,
        ~italic: option(bool)=?,
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
    foreground: int,
    background: int,
    bold: bool,
    italic: bool,
  };

  let default = {foreground: 1, background: 0, bold: false, italic: false};
};

module Selector = {
  type t = {
    scopes: Scopes.t,
    style: TokenStyle.t,
  };

  let create = (~style=TokenStyle.default, ~scopes, ()) => {scopes, style};
};
