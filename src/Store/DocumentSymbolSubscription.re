module Core = Oni_Core;
module Model = Oni_Model;

module Actions = Model.Actions;
module Subscription = Core.Subscription;
module Log = (
  val Core.Log.withNamespace("Oni2.Store.DocumentSymbolSubscription")
);
module LanguageFeatures = Feature_LanguageSupport.LanguageFeatures;

module Provider = {
  type action = Actions.t;
  type params = {
    buffer: Core.Buffer.t,
    languageFeatures: LanguageFeatures.t,
    onUpdate: list(Actions.menuItem) => unit,
  };

  let start =
      (~id, ~params as {languageFeatures, onUpdate, buffer}, ~dispatch as _) => {
    Log.debug("Starting: " ++ id);

    let promise =
      LanguageFeatures.requestDocumentSymbol(~buffer, languageFeatures);

    Lwt.on_success(
      promise,
      items => {
        open LanguageFeatures;

        let docSymbolToMenuItem = (docSymbol: DocumentSymbol.t) => {
          Actions.{
            category: Some(DocumentSymbol.(docSymbol.name)),
            name: DocumentSymbol.(docSymbol.detail),
            command: () =>
              Model.Actions.OpenFileByPath(
                Core.Buffer.getUri(buffer) |> Core.Uri.toFileSystemPath,
                None,
                Some(DocumentSymbol.(docSymbol.range.start)),
              ),
            icon: None,
            highlight: [],
          };
        };

        items |> List.map(docSymbolToMenuItem) |> onUpdate;
      },
    );
  };

  let update = (~id as _, ~params as _, ~dispatch as _) => {
    ();
      // TODO: Anything we should be doing here?
  };

  let dispose = (~id) => {
    Log.debug("Disposing: " ++ id);
  };
};

let create = (~id, ~onUpdate, ~languageFeatures, ~buffer) =>
  Subscription.create(
    id,
    (module Provider),
    {onUpdate, languageFeatures, buffer},
  );
