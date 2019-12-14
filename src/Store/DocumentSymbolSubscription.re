module Core = Oni_Core;
module Model = Oni_Model;

module Actions = Model.Actions;
module Subscription = Core.Subscription;
module Log = (val Core.Log.withNamespace("Oni2.DocumentSymbolSubscription"));

module Provider = {
  type action = Actions.t;
  type params = {
    buffer: Core.Buffer.t,
    languageFeatures: Model.LanguageFeatures.t,
    onUpdate: list(Actions.menuItem) => unit,
  };

  let start =
      (~id, ~params as {languageFeatures, onUpdate, buffer}, ~dispatch as _) => {
    Log.info("Starting DocumentSymbol subscription " ++ id);

    let promise =
      Model.LanguageFeatures.requestDocumentSymbol(~buffer, languageFeatures);

    Lwt.on_success(
      promise,
      items => {
        open Model.LanguageFeatures;

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
    Log.info("Disposing: " ++ id);
  };
};

let create = (~id, ~onUpdate, ~languageFeatures, ~buffer) =>
  Subscription.create(
    id,
    (module Provider),
    {onUpdate, languageFeatures, buffer},
  );
