open EditorCoreTypes;

module type S = {
  type msg;
  type model;

  let update: (msg, model) => model;

  let create:
    (
      ~trigger: Exthost.CompletionContext.t,
      ~buffer: Oni_Core.Buffer.t,
      ~base: string,
      ~location: CharacterPosition.t
    ) =>
    option(model);

  let items: model => result(list(Exthost.SuggestItem.t), string);

  let sub:
    (~selectedItem: option(Exthost.SuggestItem.t), model) =>
    Isolinear.Sub.t(msg);
};

type provider('model, 'msg) = (module S with
                                  type model = 'model and type msg = 'msg);

type exthostModel = result(list(Exthost.SuggestItem.t), string);
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
    });

module ExthostCompletionProvider =
       (Config: {let handle: int;})
       : (S with type model = exthostModel and type msg = exthostMsg) => {
  // Functor-supplied details
  let providerHandle = Config.handle;

  type msg = exthostMsg;
  type model = exthostModel;

  let create = (~trigger as _, ~buffer as _, ~base as _, ~location as _) =>
    None;

  let update = (msg, model) =>
    switch (msg) {
    // TODO: Handle incomplete result
    | ResultAvailable({handle, result}) when handle == providerHandle =>
      Ok(Exthost.SuggestResult.(result.completions))
    | DetailsAvailable({handle, item}) when handle == providerHandle =>
      model
      |> Result.map(items => {
           items
           |> List.map((previousItem: Exthost.SuggestItem.t) =>
                if (previousItem.chainedCacheId == item.chainedCacheId) {
                  item;
                } else {
                  previousItem;
                }
              )
         })
    | DetailsError({handle, errorMsg}) when handle == providerHandle =>
      Error(errorMsg)
    | _ => model
    };

  let items = model => model;

  let sub = (~selectedItem as _, _model) => Isolinear.Sub.none;
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
