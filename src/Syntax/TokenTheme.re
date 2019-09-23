/*
 TokenTheme.re

 Thin wrapper around TextmateTheme.t that adds cached
 */

open Textmate;

type t = {
  useCache: bool,
  cache: Hashtbl.t(string, ThemeScopes.ResolvedStyle.t),
  theme: Textmate.Theme.t,
};

let create = (~useCache=true, theme: Textmate.Theme.t) => {
  let cache = Hashtbl.create(1024);

  {cache, theme, useCache};
};

let empty = create(Textmate.Theme.empty);

let match = (v: t, scopes: string) =>
  if (!v.useCache) {
    Textmate.Theme.match(v.theme, scopes);
  } else {
    switch (Hashtbl.find_opt(v.cache, scopes)) {
    | Some(v) => v
    | None =>
      let resolvedStyle = Textmate.Theme.match(v.theme, scopes);
      Hashtbl.add(v.cache, scopes, resolvedStyle);
      resolvedStyle;
    };
  };
