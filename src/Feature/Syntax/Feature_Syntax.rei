open EditorCoreTypes;
open Oni_Core;

let highlight:
  (
    ~scope: string,
    ~theme: Oni_Syntax.TokenTheme.t,
    ~grammars: Oni_Syntax.GrammarRepository.t,
    array(string)
  ) =>
  array(list(ThemeToken.t));

[@deriving show({with_path: false})]
type msg;

type outmsg =
  | Nothing
  | ServerError(string);

type t;

let empty: t;

let getTokens:
  (~bufferId: int, ~line: EditorCoreTypes.LineNumber.t, t) =>
  list(ThemeToken.t);

let getSyntaxScope:
  (~bytePosition: BytePosition.t, ~bufferId: int, t) => SyntaxScope.t;

let getAt:
  (~bufferId: int, ~bytePosition: EditorCoreTypes.BytePosition.t, t) =>
  option(ThemeToken.t);

let setTokensForLine:
  (~bufferId: int, ~line: int, ~tokens: list(ThemeToken.t), t) => t;

let handleUpdate:
  (
    ~grammars: Oni_Syntax.GrammarRepository.t,
    ~scope: string,
    ~theme: Oni_Syntax.TokenTheme.t,
    ~config: Config.resolver,
    ~bufferUpdate: BufferUpdate.t,
    ~markerUpdate: MarkerUpdate.t,
    t
  ) =>
  t;

let update: (t, msg) => (t, outmsg);

let isSyntaxServerRunning: t => bool;

// [ignore(~bufferId, syntax)] marks a buffer to be ignored.
// The only syntax highlight adjustment will come from explicit
// calls to [setTokensForLine];
let ignore: (~bufferId: int, t) => t;

module Tokens: {
  let getAt:
    (~byteIndex: ByteIndex.t, list(ThemeToken.t)) => option(ThemeToken.t);
};

module Effect: {
  let bufferUpdate:
    (~bufferUpdate: BufferUpdate.t, t) => Isolinear.Effect.t(unit);
};

let subscription:
  (
    ~buffers: Feature_Buffers.model,
    ~config: Config.resolver,
    ~grammarInfo: Exthost.GrammarInfo.t,
    ~languageInfo: Exthost.LanguageInfo.t,
    ~setup: Setup.t,
    ~tokenTheme: Oni_Syntax.TokenTheme.t,
    ~bufferVisibility: list((Buffer.t, list(Range.t))),
    t
  ) =>
  Isolinear.Sub.t(msg);

module Contributions: {let configuration: list(Config.Schema.spec);};
