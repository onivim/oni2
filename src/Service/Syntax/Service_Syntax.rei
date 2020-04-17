open EditorCoreTypes;
open Oni_Core;

[@deriving show({with_path: false})]
type msg =
  | ServerStarted(Oni_Syntax_Client.t)
  | ServerClosed
  | ReceivedHighlights(list(Oni_Syntax.Protocol.TokenUpdate.t));

module Sub: {
  let create:
    (~languageInfo: Oni_Extensions.LanguageInfo.t, ~setup: Oni_Core.Setup.t,
    ) =>
    Isolinear.Sub.t(msg);
};

module Effect: {
  let bufferUpdate:
    (
      option(Oni_Syntax_Client.t),
      BufferUpdate.t,
      array(string),
      option(string)
    ) =>
    Isolinear.Effect.t(msg);

  // TODO:
  // These effects should not be necessary. We should change our subscription
  // granularity to per-buffer, and handle synchronization of open/visible
  // buffers, themes, and configuration via that subscription.
  let bufferEnter:
    (option(Oni_Syntax_Client.t), int, option(string)) =>
    Isolinear.Effect.t(msg);

  let configurationChange:
    (option(Oni_Syntax_Client.t), Configuration.t) => Isolinear.Effect.t(msg);

  let themeChange:
    (option(Oni_Syntax_Client.t), Oni_Syntax.TokenTheme.t) =>
    Isolinear.Effect.t(msg);

  let visibilityChanged:
    (option(Oni_Syntax_Client.t), list((int, list(Range.t)))) =>
    Isolinear.Effect.t(msg);
};
