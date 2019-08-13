/*
TextMateScopes
*/

/* 
  [scope] is a list representation of TextMate scopes.
  For example, "source.js" would be represented as ["source", "js"]
*/

module Scope {
    type t = list(string);

    let ofString = (s) => String.split_on_char('.', s);
}
module Scopes {
    /*
      In general, tokens have multiple scopes. For example,
      a token in a function might have the follow scopes:
      - "source.js"
      - "meta.definition.function.js"
      - "entity.name.function.js"

      The [t] type models this, as each bullet is a [scope].
    */
    type t = list(Scope.t);
}
