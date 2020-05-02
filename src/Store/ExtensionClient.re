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

// TODO: Bring back language features
/*module ExtensionCompletionProvider = {
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
  */

let create = (~config, ~extensions, ~setup: Setup.t) => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let extensionInfo =
    extensions
    |> List.map(
         ({manifest, path, _}: Exthost.Extension.Scanner.ScanResult.t) =>
         Exthost.Extension.InitData.Extension.ofManifestAndPath(
           manifest,
           path,
         )
       );

  let _onRegisterDefinitionProvider = (client, provider) => {
    ()// TODO
      //    let id =
      //      Protocol.BasicProvider.("exthost." ++ string_of_int(provider.id));
      //    let definitionProvider =
      //      ExtensionDefinitionProvider.create(client, provider);
      //
      //    dispatch(
      //      Actions.LanguageFeature(
      //        LanguageFeatures.DefinitionProviderAvailable(id, definitionProvider),
      //      ),
      ;
      //    );
  };

  let _onRegisterDocumentSymbolProvider = (client, provider) => {
    ()// TODO
      //    let id =
      //      Protocol.DocumentSymbolProvider.(
      //        "exthost." ++ string_of_int(provider.id)
      //      );
      //    let documentSymbolProvider =
      //      ExtensionDocumentSymbolProvider.create(client, provider);
      //
      //    dispatch(
      //      Actions.LanguageFeature(
      //        LanguageFeatures.DocumentSymbolProviderAvailable(
      //          id,
      //          documentSymbolProvider,
      //        ),
      //      ),
      ;
      //    );
  };

  let _onRegisterReferencesProvider = (client, provider) => {
    ()//    let id =
      //      Protocol.BasicProvider.("exthost." ++ string_of_int(provider.id));
      //    let findAllReferencesProvider =
      //      ExtensionFindAllReferencesProvider.create(client, provider);
      //
      //    dispatch(
      //      Actions.LanguageFeature(
      //        LanguageFeatures.FindAllReferencesProviderAvailable(
      //          id,
      //          findAllReferencesProvider,
      //        ),
      //      ),
      ;
      //    );
  };

  let _onRegisterDocumentHighlightProvider = (client, provider) => {
    ()// TODO
      //    let id =
      //      Protocol.BasicProvider.("exthost." ++ string_of_int(provider.id));
      //    let documentHighlightProvider =
      //      ExtensionDocumentHighlightProvider.create(client, provider);
      //
      //    dispatch(
      //      Actions.LanguageFeature(
      //        LanguageFeatures.DocumentHighlightProviderAvailable(
      //          id,
      //          documentHighlightProvider,
      //        ),
      //      ),
      ;
      //    );
  };

  let _onRegisterSuggestProvider = (client, provider) => {
    ()// TODO: Implement
      //    let id =
      //      Protocol.SuggestProvider.("exthost." ++ string_of_int(provider.id));
      //    let completionProvider =
      //      ExtensionCompletionProvider.create(client, provider);
      //
      //    dispatch(
      //      Actions.LanguageFeature(
      //        LanguageFeatures.CompletionProviderAvailable(id, completionProvider),
      //      ),
      ;
      //    );
  };
  let onDiagnosticsChangeMany =
      (owner: string, entries: list(Exthost.Msg.Diagnostics.entry)) => {
    let protocolDiagToDiag: Exthost.Diagnostic.t => Diagnostic.t =
      d => {
        let range = Exthost.OneBasedRange.toRange(d.range);
        let message = d.message;
        Diagnostic.create(~range, ~message, ());
      };

    let f = (d: Exthost.Msg.Diagnostics.entry) => {
      let diagnostics = List.map(protocolDiagToDiag, snd(d));
      let uri = fst(d);
      Actions.DiagnosticsSet(uri, owner, diagnostics);
    };

    entries |> List.map(f) |> List.iter(a => dispatch(a));
  };
  open Exthost;
  open Exthost.Extension;
  open Exthost.Msg;

  let handler = msg =>
    switch (msg) {
    //    TODO
    //    | SCM(msg) =>
    //      Feature_SCM.handleExtensionMessage(
    //        ~dispatch=msg => dispatch(Actions.SCM(msg)),
    //        msg,
    //      )

    | Diagnostics(Clear({owner})) =>
      dispatch(Actions.DiagnosticsClear(owner));
      None;
    | Diagnostics(ChangeMany({owner, entries})) =>
      onDiagnosticsChangeMany(owner, entries);
      None;

    | DocumentContentProvider(RegisterTextContentProvider({handle, scheme})) =>
      dispatch(NewTextContentProvider({handle, scheme}));
      None;

    | DocumentContentProvider(UnregisterTextContentProvider({handle})) =>
      dispatch(LostTextContentProvider({handle: handle}));
      None;

    | Decorations(RegisterDecorationProvider({handle, label})) =>
      dispatch(NewDecorationProvider({handle, label}));
      None;
    | Decorations(UnregisterDecorationProvider({handle})) =>
      dispatch(LostDecorationProvider({handle: handle}));
      None;
    | Decorations(DecorationsDidChange({handle, uris})) =>
      dispatch(DecorationsChanged({handle, uris}));
      None;

    | ExtensionService(DidActivateExtension({extensionId, _})) =>
      dispatch(
        Actions.Extension(Oni_Model.Extensions.Activated(extensionId)),
      );
      None;

    | MessageService(ShowMessage({severity, message, extensionId})) =>
      dispatch(ExtMessageReceived({severity, message, extensionId}));
      None;

    | StatusBar(SetEntry({id, text, source, alignment, priority})) =>
      dispatch(
        Actions.StatusBarAddItem(
          StatusBarModel.Item.create(~id, ~text, ~alignment, ~priority, ()),
        ),
      );
      None;

    | TerminalService(msg) =>
      Service_Terminal.handleExtensionMessage(msg);
      None;
    | _ => None
    };

  let parentPid = Luv.Pid.getpid();
  let name = Printf.sprintf("exthost-client-%s", parentPid |> string_of_int);
  let namedPipe = name |> NamedPipe.create;

  let tempDir = Filename.get_temp_dir_name();

  let logsLocation = tempDir |> Uri.fromPath;
  let logFile =
    Filename.temp_file(~temp_dir=tempDir, "onivim2", "exthost.log")
    |> Uri.fromPath;

  let initData =
    InitData.create(
      ~version="9.9.9", // TODO
      ~parentPid,
      ~logsLocation,
      ~logFile,
      extensionInfo,
    );

  let onError = err => {
    Log.error(err);
  };

  let client =
    Exthost.Client.start(
      ~initialConfiguration=
        Feature_Configuration.toExtensionConfiguration(
          config,
          extensions,
          setup,
        ),
      ~namedPipe,
      ~initData,
      ~handler,
      ~onError,
      (),
    );

  (client, stream);
};
