open Oni_Core;
open Utility;
open EditorCoreTypes;

type completionState =
  | Complete
  | Incomplete;

module type S = {
  type msg;
  type model;

  type outmsg =
    | Nothing
    | ProviderError(string);

  let update: (~isFuzzyMatching: bool, msg, model) => (model, outmsg);

  let create:
    (
      ~config: Oni_Core.Config.resolver,
      ~extensions: Feature_Extensions.model,
      ~languageConfiguration: LanguageConfiguration.t,
      ~trigger: Exthost.CompletionContext.t,
      ~buffer: Oni_Core.Buffer.t,
      ~base: string,
      ~location: CharacterPosition.t
    ) =>
    option(model);

  let items: model => (completionState, list(CompletionItem.t));

  let handle: unit => option(int);

  let sub:
    (
      ~client: Exthost.Client.t,
      ~context: Exthost.CompletionContext.t,
      ~position: CharacterPosition.t,
      ~buffer: Oni_Core.Buffer.t,
      ~selectedItem: option(CompletionItem.t),
      model
    ) =>
    Isolinear.Sub.t(msg);
};

type provider('model, 'msg) = (module S with
                                  type model = 'model and type msg = 'msg);

type exthostModel = {
  isComplete: bool,
  completions: list(CompletionItem.t),
};

[@deriving show]
type exthostMsg =
  | ResultAvailable({
      handle: int,
      result: Exthost.SuggestResult.t,
    })
  | DetailsAvailable({
      handle: int,
      item: Exthost.SuggestItem.t,
    })
  | DetailsError({
      handle: int,
      errorMsg: string,
    })
  | ResultError({
      handle: int,
      errorMsg: string,
    });

module ExthostCompletionProvider =
       (
         Config: {
           let handle: int;
           let selector: Exthost.DocumentSelector.t;
           let supportsResolve: bool;
         },
       )
       : (S with type model = exthostModel and type msg = exthostMsg) => {
  // Functor-supplied details
  let providerHandle = Config.handle;

  type msg = exthostMsg;
  type model = exthostModel;

  type outmsg =
    | Nothing
    | ProviderError(string);

  let create =
      (
        ~config as _,
        ~extensions as _,
        ~languageConfiguration as _,
        ~trigger as _,
        ~buffer,
        ~base as _,
        ~location as _,
      ) =>
    if (!Exthost.DocumentSelector.matchesBuffer(~buffer, Config.selector)) {
      None;
    } else {
      Some({completions: [], isComplete: true});
    };

  let handle = () => Some(providerHandle);

  let update = (~isFuzzyMatching, msg, model) =>
    switch (msg) {
    | ResultAvailable({handle, result}) when handle == providerHandle =>
      let isComplete = !result.isIncomplete;
      let completions =
        Exthost.SuggestResult.(result.completions)
        |> List.map(
             CompletionItem.create(~isFuzzyMatching, ~handle=providerHandle),
           );
      ({isComplete, completions}, Nothing);
    | DetailsAvailable({handle, item}) when handle == providerHandle =>
      let completions' =
        model.completions
        |> List.map((previousItem: CompletionItem.t) =>
             if (previousItem.chainedCacheId == item.chainedCacheId) {
               CompletionItem.create(
                 ~isFuzzyMatching=previousItem.isFuzzyMatching,
                 ~handle=providerHandle,
                 item,
               );
             } else {
               previousItem;
             }
           );
      ({...model, completions: completions'}, Nothing);
    | DetailsError({handle, errorMsg}) when handle == providerHandle => (
        model,
        ProviderError(errorMsg),
      )
    | ResultError({handle, errorMsg}) when handle == providerHandle => (
        model,
        ProviderError(errorMsg),
      )
    | _ => (model, Nothing)
    };

  let items = ({isComplete, completions}) => (
    isComplete ? Complete : Incomplete,
    completions,
  );

  let sub =
      (
        ~client,
        ~context,
        ~position,
        ~buffer,
        ~selectedItem: option(CompletionItem.t),
        _model,
      ) => {
    let itemsSub =
      Service_Exthost.Sub.completionItems(
        ~context,
        ~handle=providerHandle,
        ~buffer,
        ~position,
        ~toMsg=
          suggestResult => {
            switch (suggestResult) {
            | Ok(v) => ResultAvailable({handle: providerHandle, result: v})
            | Error(errorMsg) =>
              ResultError({handle: providerHandle, errorMsg})
            }
          },
        client,
      );

    selectedItem
    |> OptionEx.flatMap((selected: CompletionItem.t) =>
         if (selected.handle == Some(providerHandle) && Config.supportsResolve) {
           selected.chainedCacheId
           |> Option.map(chainedCacheId => {
                let resolveSub =
                  Service_Exthost.Sub.completionItem(
                    ~handle=providerHandle,
                    ~chainedCacheId,
                    ~toMsg=
                      fun
                      | Ok(item) =>
                        DetailsAvailable({handle: providerHandle, item})
                      | Error(errorMsg) =>
                        DetailsError({handle: providerHandle, errorMsg}),
                    client,
                  );

                [itemsSub, resolveSub] |> Isolinear.Sub.batch;
              });
         } else {
           None;
         }
       )
    |> Option.value(~default=itemsSub);
  };
};
let exthost:
  (
    ~selector: Exthost.DocumentSelector.t,
    ~handle: int,
    ~supportsResolve: bool
  ) =>
  provider(exthostModel, exthostMsg) =
  (~selector, ~handle, ~supportsResolve) => {
    let provider:
      module S with type model = exthostModel and type msg = exthostMsg =
      (module
       ExthostCompletionProvider({
         let handle = handle;
         let selector = selector;
         let supportsResolve = supportsResolve;
       }));
    provider;
  };

type keywordModel = list(CompletionItem.t);
[@deriving show]
type keywordMsg = unit;

module KeywordCompletionProvider =
       (())
       : (S with type msg = keywordMsg and type model = keywordModel) => {
  type msg = keywordMsg;
  type model = keywordModel;

  type outmsg =
    | Nothing
    | ProviderError(string);

  let handle = () => None;

  let create =
      (
        ~config,
        ~extensions as _,
        ~languageConfiguration: LanguageConfiguration.t,
        ~trigger: Exthost.CompletionContext.t,
        ~buffer,
        ~base: string,
        ~location: CharacterPosition.t,
      ) => {
    ignore(trigger);
    ignore(base);
    ignore(location);

    if (!CompletionConfig.wordBasedSuggestions.get(config)) {
      None;
    } else {
      let keywords =
        Feature_Keywords.keywords(~languageConfiguration, ~buffer);

      let keywords =
        keywords
        |> List.mapi((idx, keyword) => {
             CompletionItem.keyword(
               ~sortOrder=idx,
               ~isFuzzyMatching=base != "",
               keyword,
             )
           });

      Some(keywords);
    };
  };

  let update = (~isFuzzyMatching as _, _msg: msg, model: model) => (
    model,
    Nothing,
  );

  let items = model => (Complete, model);

  let sub =
      (
        ~client as _,
        ~context as _,
        ~position as _,
        ~buffer as _,
        ~selectedItem as _,
        _model,
      ) => Isolinear.Sub.none;
};

let keyword: provider(keywordModel, keywordMsg) = {
  (module
   KeywordCompletionProvider({}));
};

type snippetModel = {
  items: list(CompletionItem.t),
  isComplete: bool,
  filePaths: list(FpExp.t(FpExp.absolute)),
  fileType: string,
  sortOrder: [ | `Top | `Inline | `Bottom | `Hidden],
};
[@deriving show]
type snippetMsg =
  | SnippetsAvailable(list(Service_Snippets.SnippetWithMetadata.t));

module SnippetCompletionProvider =
       (())
       : (S with type msg = snippetMsg and type model = snippetModel) => {
  type msg = snippetMsg;
  type model = snippetModel;

  type outmsg =
    | Nothing
    | ProviderError(string);

  let handle = () => None;

  let create =
      (
        ~config,
        ~extensions,
        ~languageConfiguration as _,
        ~trigger: Exthost.CompletionContext.t,
        ~buffer,
        ~base: string,
        ~location: CharacterPosition.t,
      ) => {
    ignore(trigger);
    ignore(base);
    ignore(location);

    let sortOrder = CompletionConfig.snippetSuggestions.get(config);
    if (sortOrder == `Hidden) {
      None;
    } else {
      let fileType = buffer |> Buffer.getFileType |> Buffer.FileType.toString;

      let snippetFilePaths =
        Feature_Extensions.snippetFilePaths(~fileType, extensions);

      Some({
        filePaths: snippetFilePaths,
        fileType,
        items: [],
        isComplete: false,
        sortOrder,
      });
    };
  };

  let update = (~isFuzzyMatching, msg, model: model) => {
    Service_Snippets.(
      switch (msg) {
      | SnippetsAvailable(snippets) =>
        let items =
          snippets
          |> List.map((snippet: SnippetWithMetadata.t) => {
               CompletionItem.snippet(
                 ~isFuzzyMatching,
                 ~prefix=snippet.prefix,
                 snippet.snippet,
               )
             });
        ({...model, items, isComplete: true}, Nothing);
      }
    );
  };

  let items = ({items, isComplete, _}: model) => (
    isComplete ? Complete : Incomplete,
    items,
  );

  let sub =
      (
        ~client as _,
        ~context as _,
        ~position as _,
        ~buffer as _,
        ~selectedItem as _,
        model,
      ) => {
    let filePaths = model.filePaths;
    let uniqueId = "Feature_LanguageSupport.SnippetCompletionProvider";

    let toMsg = snippets => SnippetsAvailable(snippets);
    Service_Snippets.Sub.snippetFromFiles(
      ~uniqueId,
      ~fileType=model.fileType,
      ~filePaths,
      toMsg,
    );
  };
};

let snippet: provider(snippetModel, snippetMsg) = {
  (module
   SnippetCompletionProvider({}));
};
