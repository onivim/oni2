open Oni_Core;
open Utility;
open EditorCoreTypes;

module type S = {
  type msg;
  type model;

  let update: (~isFuzzyMatching: bool, msg, model) => model;

  let create:
    (
      ~config: Oni_Core.Config.resolver,
      ~languageConfiguration: LanguageConfiguration.t,
      ~trigger: Exthost.CompletionContext.t,
      ~buffer: Oni_Core.Buffer.t,
      ~base: string,
      ~location: CharacterPosition.t
    ) =>
    option(model);

  let items: model => result(list(CompletionItem.t), string);

  let handle: unit => option(int);

  let sub:
    (
      ~client: Exthost.Client.t,
      ~position: CharacterPosition.t,
      ~buffer: Oni_Core.Buffer.t,
      ~selectedItem: option(CompletionItem.t),
      model
    ) =>
    Isolinear.Sub.t(msg);
};

type provider('model, 'msg) = (module S with
                                  type model = 'model and type msg = 'msg);

type exthostModel = result(list(CompletionItem.t), string);
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

  let create =
      (
        ~config as _,
        ~languageConfiguration as _,
        ~trigger as _,
        ~buffer,
        ~base as _,
        ~location as _,
      ) =>
    if (!Exthost.DocumentSelector.matchesBuffer(~buffer, Config.selector)) {
      None;
    } else {
      Some(Ok([]));
    };

  let handle = () => Some(providerHandle);

  let update = (~isFuzzyMatching, msg, model) =>
    switch (msg) {
    // TODO: Handle incomplete result
    | ResultAvailable({handle, result}) when handle == providerHandle =>
      let completions =
        Exthost.SuggestResult.(result.completions)
        |> List.map(
             CompletionItem.create(~isFuzzyMatching, ~handle=providerHandle),
           );
      Ok(completions);
    | DetailsAvailable({handle, item}) when handle == providerHandle =>
      model
      |> Result.map(items => {
           items
           |> List.map((previousItem: CompletionItem.t) =>
                if (previousItem.chainedCacheId == item.chainedCacheId) {
                  CompletionItem.create(
                    ~isFuzzyMatching,
                    ~handle=providerHandle,
                    item,
                  );
                } else {
                  previousItem;
                }
              )
         })
    | DetailsError({handle, errorMsg}) when handle == providerHandle =>
      Error(errorMsg)
    | ResultError({handle, errorMsg}) when handle == providerHandle =>
      Error(errorMsg)
    | _ => model
    };

  let items = model => model;

  let sub =
      (
        ~client,
        ~position,
        ~buffer,
        ~selectedItem: option(CompletionItem.t),
        _model,
      ) => {
    let itemsSub =
      Service_Exthost.Sub.completionItems(
        // TODO: proper trigger kind
        ~context=
          Exthost.CompletionContext.{
            triggerKind: Invoke,
            triggerCharacter: None,
          },
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

  let handle = () => None;

  let create =
      (
        ~config,
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

  let update = (~isFuzzyMatching as _, _msg: msg, model: model) => model;

  let items = model => Ok(model);

  let sub =
      (~client as _, ~position as _, ~buffer as _, ~selectedItem as _, _model) => Isolinear.Sub.none;
};

let keyword: provider(keywordModel, keywordMsg) = {
  (module
   KeywordCompletionProvider({}));
};
