open EditorCoreTypes;
open Oni_Core;
open Oni_Syntax;

[@deriving show({with_path: false})]
type serverMsg =
  | ServerStarted(Oni_Syntax_Client.t)
  | ServerFailedToStart(string)
  | ServerClosed;

type bufferMsg =
  | ReceivedHighlights(list(Oni_Syntax.Protocol.TokenUpdate.t));

module Sub: {
  let server:
    (
      ~configuration: Configuration.t,
      ~languageInfo: Oni_Extensions.LanguageInfo.t,
      ~setup: Oni_Core.Setup.t,
      ~tokenTheme: TokenTheme.t
    ) =>
    Isolinear.Sub.t(serverMsg);

  let buffer:
    (
      ~client: Oni_Syntax_Client.t,
      ~buffer: Oni_Core.Buffer.t,
      ~visibleRanges: list(Range.t)
    ) =>
    Isolinear.Sub.t(bufferMsg);
};
