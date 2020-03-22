open EditorCoreTypes;
open Oni_Core;
open Oni_Model;

module Log = (val Log.withNamespace("Oni2.Extension.ClientStore"));

open Oni_Extensions;
module Extensions = Oni_Extensions;
module Protocol = Extensions.ExtHostProtocol;
module CompletionItem = Feature_LanguageSupport.CompletionItem;
module Diagnostic = Feature_LanguageSupport.Diagnostic;
module LanguageFeatures = Feature_LanguageSupport.LanguageFeatures;

module Workspace = Protocol.Workspace;

module ExtensionCompletionProvider = {
  let suggestionItemToCompletionItem:
    Protocol.SuggestionItem.t => CompletionItem.t =
    suggestion => {
      let completionKind =
        Option.bind(suggestion.kind, CompletionItemKind.ofInt);

      {
        label: suggestion.label,
        kind: completionKind,
        detail: suggestion.detail,
      };
    };

  let suggestionsToCompletionItems:
    option(Protocol.Suggestions.t) => list(CompletionItem.t) =
    fun
    | Some(suggestions) =>
      List.map(suggestionItemToCompletionItem, suggestions)
    | None => [];

  let create =
      (
        client: ExtHostClient.t,
        {id, selector}: Protocol.SuggestProvider.t,
        (buffer, _completionMeet, location),
      ) =>
    ProviderUtility.runIfSelectorPasses(
      ~buffer,
      ~selector,
      () => {
        let uri = Buffer.getUri(buffer);
        let position = Protocol.OneBasedPosition.ofPosition(location);
        ExtHostClient.provideCompletions(id, uri, position, client)
        |> Lwt.map(suggestionsToCompletionItems);
      },
    );
};

module ExtensionDefinitionProvider = {
  let definitionToModel = def => {
    let Protocol.DefinitionLink.{uri, range, originSelectionRange} = def;
    let Range.{start, _} = Protocol.OneBasedRange.toRange(range);

    let originRange =
      originSelectionRange |> Option.map(Protocol.OneBasedRange.toRange);

    LanguageFeatures.DefinitionResult.create(
      ~originRange,
      ~uri,
      ~location=start,
    );
  };

  let create =
      (client, {id, selector}: Protocol.BasicProvider.t, (buffer, location)) => {
    ProviderUtility.runIfSelectorPasses(
      ~buffer,
      ~selector,
      () => {
        let uri = Buffer.getUri(buffer);
        let position = Protocol.OneBasedPosition.ofPosition(location);
        ExtHostClient.provideDefinition(id, uri, position, client)
        |> Lwt.map(definitionToModel);
      },
    );
  };
};

module ExtensionDocumentHighlightProvider = {
  let definitionToModel = (highlights: list(Protocol.DocumentHighlight.t)) => {
    highlights
    |> List.map(highlights =>
         Protocol.OneBasedRange.toRange(
           Protocol.DocumentHighlight.(highlights.range),
         )
       );
  };

  let create =
      (client, {id, selector}: Protocol.BasicProvider.t, (buffer, location)) => {
    ProviderUtility.runIfSelectorPasses(
      ~buffer,
      ~selector,
      () => {
        let uri = Buffer.getUri(buffer);
        let position = Protocol.OneBasedPosition.ofPosition(location);

        ExtHostClient.provideDocumentHighlights(id, uri, position, client)
        |> Lwt.map(definitionToModel);
      },
    );
  };
};

module ExtensionFindAllReferencesProvider = {
  let create =
      (client, {id, selector}: Protocol.BasicProvider.t, (buffer, location)) => {
    ProviderUtility.runIfSelectorPasses(
      ~buffer,
      ~selector,
      () => {
        let uri = Buffer.getUri(buffer);
        let position = Protocol.OneBasedPosition.ofPosition(location);

        ExtHostClient.provideReferences(id, uri, position, client);
      },
    );
  };
};

module ExtensionDocumentSymbolProvider = {
  let create =
      (client, {id, selector, _}: Protocol.DocumentSymbolProvider.t, buffer) => {
    ProviderUtility.runIfSelectorPasses(
      ~buffer,
      ~selector,
      () => {
        let uri = Buffer.getUri(buffer);
        ExtHostClient.provideDocumentSymbols(id, uri, client);
      },
    );
  };
};

let create = (~extensions, ~setup: Setup.t) => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let manifests =
    List.map((ext: ExtensionScanner.t) => ext.manifest, extensions);

  let defaults = Configuration.Model.ofExtensions(manifests);
  let keys = ["reason_language_server.location"];

  let contents =
    `Assoc([
      (
        "reason_language_server",
        `Assoc([("location", `String(setup.rlsPath))]),
      ),
      (
        "terminal",
        `Assoc([
          (
            "integrated",
            `Assoc([
              (
                "env",
                `Assoc([
                  ("windows", `Null),
                  ("linux", `Null),
                  ("osx", `Null),
                ]),
              ),
            ]),
          ),
        ]),
      ),
    ]);
  let user = Configuration.Model.create(~keys, contents);

  let initialConfiguration = Configuration.create(~defaults, ~user, ());

  let onExtHostClosed = () => Log.debug("ext host closed");

  let extensionInfo =
    extensions
    |> List.map(ext =>
         Extensions.ExtHostInitData.ExtensionInfo.ofScannedExtension(ext)
       );

  let onDiagnosticsClear = owner => {
    dispatch(Actions.DiagnosticsClear(owner));
  };

  let onDiagnosticsChangeMany =
      (diagCollection: Protocol.DiagnosticsCollection.t) => {
    let protocolDiagToDiag: Protocol.Diagnostic.t => Diagnostic.t =
      d => {
        let range = Protocol.OneBasedRange.toRange(d.range);
        let message = d.message;
        Diagnostic.create(~range, ~message, ());
      };

    let f = (d: Protocol.Diagnostics.t) => {
      let diagnostics = List.map(protocolDiagToDiag, snd(d));
      let uri = fst(d);
      Actions.DiagnosticsSet(uri, diagCollection.name, diagnostics);
    };

    diagCollection.perFileDiagnostics
    |> List.map(f)
    |> List.iter(a => dispatch(a));
  };

  let onStatusBarSetEntry = ((id, text, alignment, priority)) => {
    dispatch(
      Actions.StatusBarAddItem(
        StatusBarModel.Item.create(
          ~id,
          ~text,
          ~alignment=StatusBarModel.Alignment.ofInt(alignment),
          ~priority,
          (),
        ),
      ),
    );
  };

  let onRegisterDefinitionProvider = (client, provider) => {
    let id =
      Protocol.BasicProvider.("exthost." ++ string_of_int(provider.id));
    let definitionProvider =
      ExtensionDefinitionProvider.create(client, provider);

    dispatch(
      Actions.LanguageFeature(
        LanguageFeatures.DefinitionProviderAvailable(id, definitionProvider),
      ),
    );
  };

  let onRegisterDocumentSymbolProvider = (client, provider) => {
    let id =
      Protocol.DocumentSymbolProvider.(
        "exthost." ++ string_of_int(provider.id)
      );
    let documentSymbolProvider =
      ExtensionDocumentSymbolProvider.create(client, provider);

    dispatch(
      Actions.LanguageFeature(
        LanguageFeatures.DocumentSymbolProviderAvailable(
          id,
          documentSymbolProvider,
        ),
      ),
    );
  };

  let onRegisterReferencesProvider = (client, provider) => {
    let id =
      Protocol.BasicProvider.("exthost." ++ string_of_int(provider.id));
    let findAllReferencesProvider =
      ExtensionFindAllReferencesProvider.create(client, provider);

    dispatch(
      Actions.LanguageFeature(
        LanguageFeatures.FindAllReferencesProviderAvailable(
          id,
          findAllReferencesProvider,
        ),
      ),
    );
  };

  let onRegisterDocumentHighlightProvider = (client, provider) => {
    let id =
      Protocol.BasicProvider.("exthost." ++ string_of_int(provider.id));
    let documentHighlightProvider =
      ExtensionDocumentHighlightProvider.create(client, provider);

    dispatch(
      Actions.LanguageFeature(
        LanguageFeatures.DocumentHighlightProviderAvailable(
          id,
          documentHighlightProvider,
        ),
      ),
    );
  };

  let onRegisterSuggestProvider = (client, provider) => {
    let id =
      Protocol.SuggestProvider.("exthost." ++ string_of_int(provider.id));
    let completionProvider =
      ExtensionCompletionProvider.create(client, provider);

    dispatch(
      Actions.LanguageFeature(
        LanguageFeatures.CompletionProviderAvailable(id, completionProvider),
      ),
    );
  };

  let onClientMessage = (msg: ExtHostClient.msg) =>
    switch (msg) {
    | SCM(msg) =>
      Feature_SCM.handleExtensionMessage(
        ~dispatch=msg => dispatch(Actions.SCM(msg)),
        msg,
      )
    | Terminal(msg) => Service_Terminal.handleExtensionMessage(msg)

    | RegisterTextContentProvider({handle, scheme}) =>
      dispatch(NewTextContentProvider({handle, scheme}))

    | UnregisterTextContentProvider({handle}) =>
      dispatch(LostTextContentProvider({handle: handle}))

    | RegisterDecorationProvider({handle, label}) =>
      dispatch(NewDecorationProvider({handle, label}))

    | UnregisterDecorationProvider({handle}) =>
      dispatch(LostDecorationProvider({handle: handle}))

    | DecorationsDidChange({handle, uris}) =>
      dispatch(DecorationsChanged({handle, uris}))
    };

  let onOutput = Log.info;

  let onDidActivateExtension = id => {
    dispatch(Actions.Extension(Oni_Model.Extensions.Activated(id)));
  };

  let onShowMessage = message => {
    dispatch(Actions.ExtMessageReceived(message));
  };

  let initData = ExtHostInitData.create(~extensions=extensionInfo, ());

  let client =
    Extensions.ExtHostClient.start(
      ~initialConfiguration,
      ~initialWorkspace=Workspace.fromPath(Sys.getcwd()),
      ~initData,
      ~onClosed=onExtHostClosed,
      ~onStatusBarSetEntry,
      ~onDiagnosticsClear,
      ~onDiagnosticsChangeMany,
      ~onDidActivateExtension,
      ~onRegisterDefinitionProvider,
      ~onRegisterDocumentHighlightProvider,
      ~onRegisterDocumentSymbolProvider,
      ~onRegisterReferencesProvider,
      ~onRegisterSuggestProvider,
      ~onShowMessage,
      ~onOutput,
      ~dispatch=onClientMessage,
      setup,
    );

  (client, stream);
};
