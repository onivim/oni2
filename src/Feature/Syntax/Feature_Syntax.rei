open EditorCoreTypes;
open Oni_Core;

[@deriving show({with_path: false})]
type msg =
  | TokensHighlighted([@opaque] list(Oni_Syntax.Protocol.TokenUpdate.t))
  | BufferUpdated([@opaque] BufferUpdate.t);

type t;

let empty: t;

let getTokens: (~bufferId: int, ~line: Index.t, t) => list(ColorizedToken.t);

let getSyntaxScope:
    (~bufferId: int, ~line: Index.t, ~bytePosition: int, t) =>
      SyntaxScope.t;

let update: (t, msg) => t;
