/*
 TokenTheme.re

 Thin wrapper around TextmateTheme.t that adds caching
 */

open Textmate;

type t = {
  useCache: bool,
  cache: Hashtbl.t(string, ThemeScopes.ResolvedStyle.t),
  theme: Textmate.TokenTheme.t,
  commentColor: Revery.Color.t,
  constantColor: Revery.Color.t,
  entityColor: Revery.Color.t,
  functionColor: Revery.Color.t,
  keywordColor: Revery.Color.t,
  textColor: Revery.Color.t,
  typeColor: Revery.Color.t,
  variableColor: Revery.Color.t,
};


let match = (v: t, scopes: string) =>
  if (!v.useCache) {
    Textmate.TokenTheme.match(v.theme, scopes);
  } else {
    switch (Hashtbl.find_opt(v.cache, scopes)) {
    | Some(v) => v
    | None =>
      let resolvedStyle = Textmate.TokenTheme.match(v.theme, scopes);
      Hashtbl.add(v.cache, scopes, resolvedStyle);
      resolvedStyle;
    };
  };

let create = (~useCache=true, theme: Textmate.TokenTheme.t) => {
  let cache = Hashtbl.create(1024);

  let commentColor = Textmate.TokenTheme.match(theme, "comment").foreground |> Revery.Color.hex;
  let constantColor = Textmate.TokenTheme.match(theme, "constant").foreground |> Revery.Color.hex;
  let entityColor = Textmate.TokenTheme.match(theme, "entity").foreground |> Revery.Color.hex;
  let keywordColor = Textmate.TokenTheme.match(theme, "keyword").foreground |> Revery.Color.hex;
  let textColor = Textmate.TokenTheme.match(theme, "text").foreground |> Revery.Color.hex;
  let functionColor = Textmate.TokenTheme.match(theme, "entity.name.function").foreground |> Revery.Color.hex;
  let typeColor = Textmate.TokenTheme.match(theme, "storage.type").foreground |> Revery.Color.hex;
  let variableColor = Textmate.TokenTheme.match(theme, "variable.other.member").foreground |> Revery.Color.hex;

  {cache, theme, useCache, commentColor, constantColor, entityColor, keywordColor, textColor, variableColor, functionColor, typeColor};
};

let getCommentColor = (v: t) => v.commentColor;
let getConstantColor = (v: t) => v.constantColor;
let getEntityColor = (v: t) => v.entityColor;
let getKeywordColor = (v: t) => v.keywordColor;
let getTextColor = (v: t) => v.textColor;
let getVariableColor = (v: t) => v.variableColor;
let getFunctionColor = (v: t) => v.functionColor;
let getTypeColor = (v: t) => v.typeColor;

let empty = create(Textmate.TokenTheme.empty);

