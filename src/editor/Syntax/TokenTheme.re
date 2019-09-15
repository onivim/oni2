/*
 Theme.re

 Thin wrapper around TextmateTheme.t that adds cached
 */

type t = {
  useCache: bool,
  cache: Hashtbl.t(string, TextMateScopes.ResolvedStyle.t),
  theme: TextMateTheme.t,
};

let create = (~useCache=true, theme: TextMateTheme.t) => {
  let cache = Hashtbl.create(1024);

  {cache, theme, useCache};
};

let match = (v: t, scopes: string) =>
  if (!v.useCache) {
    TextMateTheme.match(v.theme, scopes);
  } else {
    switch (Hashtbl.find_opt(v.cache, scopes)) {
    | Some(v) => v
    | None =>
      let resolvedStyle = TextMateTheme.match(v.theme, scopes);
      Hashtbl.add(v.cache, scopes, resolvedStyle);
      resolvedStyle;
    };
  };
