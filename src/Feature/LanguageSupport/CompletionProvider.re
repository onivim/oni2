open EditorCoreTypes;

module type S = {
  type msg;
  type model;

  let update: (~isFuzzyMatching: bool, msg, model) => model;

  let create:
    (
      ~trigger: Exthost.CompletionContext.t,
      ~buffer: Oni_Core.Buffer.t,
      ~base: string,
      ~location: CharacterPosition.t
    ) =>
    option(model);

  let items: model => result(list(CompletionItem.t), string);

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
       (Config: {let handle: int;})
       : (S with type model = exthostModel and type msg = exthostMsg) => {
  // Functor-supplied details
  let providerHandle = Config.handle;

  type msg = exthostMsg;
  type model = exthostModel;

  let create = (~trigger as _, ~buffer as _, ~base as _, ~location as _) => {
    // TODO: Check if meet is valid?
    Some(Ok([]));
  };

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

  let sub = (~client, ~position, ~buffer, ~selectedItem as _, model) => {
    prerr_endline("SUB!");
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
          | Ok(v) =>
            prerr_endline("RESULT: " ++ Exthost.SuggestResult.show(v));
            ResultAvailable({handle: providerHandle, result: v});
          | Error(errorMsg) =>
            ResultError({handle: providerHandle, errorMsg})
          }
        },
      client,
    );
  };
};
let exthost: (~handle: int) => provider(exthostModel, exthostMsg) =
  (~handle: int) => {
    let provider:
      module S with type model = exthostModel and type msg = exthostMsg =
      (module
       ExthostCompletionProvider({
         let handle = handle;
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

  let create =
      (
        ~trigger: Exthost.CompletionContext.t,
        ~buffer,
        ~base: string,
        ~location: CharacterPosition.t,
      ) => {
    ignore(trigger);
    ignore(base);
    ignore(location);

    let keywords =
      Feature_Keywords.keywords(
        ~languageConfiguration=Oni_Core.LanguageConfiguration.default,
        ~buffer,
      );
    //    keywords |> List.iter(key => prerr_endline("KEYWORD: " ++ key));

    let keywords =
      keywords
      |> List.map(keyword => {
           CompletionItem.keyword(~isFuzzyMatching=base != "", keyword)
         });

    Some(keywords);
  };

  let update = (~isFuzzyMatching: bool, _msg: msg, model: model) => model;

  let items = model => Ok(model);

  let sub =
      (~client as _, ~position as _, ~buffer as _, ~selectedItem as _, _model) => Isolinear.Sub.none;
};

let keyword: provider(keywordModel, keywordMsg) = {
  (module
   KeywordCompletionProvider({}));
};
