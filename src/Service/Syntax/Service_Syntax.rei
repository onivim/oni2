type msg =
  | ServerStarted(Oni_Syntax_Client.t)
  | ServerClosed
  | ReceivedHighlights(list(Oni_Syntax.Protocol.TokenUpdate.t));

module Sub: {
  let create:  (~languageInfo: Oni_Extensions.LanguageInfo.t, ~setup: Oni_Core.Setup.t) => 
    Isolinear.Sub.t(msg);
};
