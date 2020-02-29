open EditorCoreTypes;
open Oni_Core;

[@deriving show({with_path: false})]
type msg =
  | ServerStarted([@opaque] Oni_Syntax_Client.t)
  | ServerStopped
  | TokensHighlighted([@opaque] list(Oni_Syntax.Protocol.TokenUpdate.t))
  | BufferUpdated([@opaque] BufferUpdate.t)
  | Service(Service_Syntax.msg);

type t;

let empty: t;

let getTokens: (~bufferId: int, ~line: Index.t, t) => list(ColorizedToken.t);

let getSyntaxScope:
  (~bufferId: int, ~line: Index.t, ~bytePosition: int, t) => SyntaxScope.t;

let update: (t, msg) => t;

let subscription:
  (
    ~enabled: bool,
    ~quitting: bool,
    ~languageInfo: Oni_Extensions.LanguageInfo.t,
    ~setup: Setup.t,
    t
  ) =>
  Isolinear.Sub.t(msg);
