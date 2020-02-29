open EditorCoreTypes;
open Oni_Core;

let highlight:
  (
    ~scope: string,
    ~theme: Oni_Syntax.TokenTheme.t,
    ~grammars: Oni_Syntax.GrammarRepository.t,
    array(string)
  ) =>
  array(list(ColorizedToken.t));

[@deriving show({with_path: false})]
type msg =
  | ServerStarted([@opaque] Oni_Syntax_Client.t)
  | ServerStopped
  | TokensHighlighted([@opaque] list(Oni_Syntax.Protocol.TokenUpdate.t))
  | BufferUpdated([@opaque] BufferUpdate.t);

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
