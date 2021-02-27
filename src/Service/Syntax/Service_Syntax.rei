open EditorCoreTypes;
open Oni_Core;
open Oni_Syntax;

[@deriving show({with_path: false})]
type serverMsg =
  | ServerStarted
  | ServerInitialized(Oni_Syntax_Client.t)
  | ServerFailedToStart(string)
  | ServerClosed;

type bufferMsg =
  | ReceivedHighlights(list(Oni_Syntax.Protocol.TokenUpdate.t));

module Effect: {
  let bufferUpdate:
    (~client: Oni_Syntax_Client.t, ~bufferUpdate: BufferUpdate.t) =>
    Isolinear.Effect.t(_);
};

module Sub: {
  let server:
    (
      ~useTreeSitter: bool,
      ~grammarInfo: Exthost.GrammarInfo.t,
      ~setup: Setup.t,
      ~tokenTheme: TokenTheme.t
    ) =>
    Isolinear.Sub.t(serverMsg);

  let buffer:
    (
      ~client: Oni_Syntax_Client.t,
      ~buffer: Buffer.t,
      ~languageInfo: Exthost.LanguageInfo.t,
      ~visibleRanges: list(Range.t)
    ) =>
    Isolinear.Sub.t(bufferMsg);
};
