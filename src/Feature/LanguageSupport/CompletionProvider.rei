open Oni_Core;
open EditorCoreTypes;

// [S] is the interface for completion providers
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
      ~languageConfiguration: LanguageConfiguration.t,
      ~trigger: Exthost.CompletionContext.t,
      ~buffer: Oni_Core.Buffer.t,
      ~base: string,
      ~location: CharacterPosition.t
    ) =>
    option(model);

  let items: model => list(CompletionItem.t);

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

type exthostModel;

[@deriving show]
type exthostMsg;

let exthost:
  (
    ~selector: Exthost.DocumentSelector.t,
    ~handle: int,
    ~supportsResolve: bool
  ) =>
  provider(exthostModel, exthostMsg);

type keywordModel;

[@deriving show]
type keywordMsg;

let keyword: provider(keywordModel, keywordMsg);
