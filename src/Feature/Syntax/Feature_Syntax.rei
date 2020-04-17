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
  | BufferUpdated([@opaque] BufferUpdate.t)
  | Service(Service_Syntax.msg);

type t;

let empty: t;

let getTokens: (~bufferId: int, ~line: Index.t, t) => list(ColorizedToken.t);

let getSyntaxScope:
  (~bufferId: int, ~line: Index.t, ~bytePosition: int, t) => SyntaxScope.t;

let setTokensForLine:
  (~bufferId: int, ~line: int, ~tokens: list(ColorizedToken.t), t) => t;

let update: (t, msg) => t;

// [ignore(~bufferId, syntax)] marks a buffer to be ignored.
// The only syntax highlight adjustment will come from explicit
// calls to [setTokensForLine];
let ignore: (~bufferId: int, t) => t;

let subscription:
  (
    ~enabled: bool,
    ~quitting: bool,
    ~languageInfo: Oni_Extensions.LanguageInfo.t,
    ~setup: Setup.t,
    ~tokenTheme: Oni_Syntax.TokenTheme.t,
    t
  ) =>
  Isolinear.Sub.t(msg);
