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
type msg;

type outmsg =
  | Nothing
  | ServerError(string);

type t;

let empty: t;

let getTokens: (~bufferId: int, ~line: Index.t, t) => list(ColorizedToken.t);

let getSyntaxScope:
  (~bufferId: int, ~line: Index.t, ~bytePosition: int, t) => SyntaxScope.t;

let setTokensForLine:
  (~bufferId: int, ~line: int, ~tokens: list(ColorizedToken.t), t) => t;

let handleUpdate:
  (
    ~grammars: Oni_Syntax.GrammarRepository.t,
    ~scope: string,
    ~theme: Oni_Syntax.TokenTheme.t,
    ~configuration: Oni_Core.Configuration.t,
    BufferUpdate.t,
    t
  ) =>
  t;

let update: (t, msg) => (t, outmsg);

let isSyntaxServerRunning: t => bool;

// [ignore(~bufferId, syntax)] marks a buffer to be ignored.
// The only syntax highlight adjustment will come from explicit
// calls to [setTokensForLine];
let ignore: (~bufferId: int, t) => t;

module Effect: {
  let bufferUpdate:
    (~bufferUpdate: BufferUpdate.t, t) => Isolinear.Effect.t(unit);
};

let subscription:
  (
    ~configuration: Configuration.t,
    ~languageInfo: Oni_Extensions.LanguageInfo.t,
    ~setup: Setup.t,
    ~tokenTheme: Oni_Syntax.TokenTheme.t,
    ~bufferVisibility: list((Buffer.t, list(Range.t))),
    t
  ) =>
  Isolinear.Sub.t(msg);
