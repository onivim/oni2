[@deriving show]
type t = {providers: list(Oni_Core.SCMModels.Provider.t)};

let initial: t;

module Effects: {
  let getOriginalUri:
    (Oni_Extensions.ExtHostClient.t, t, string, Oni_Core.Uri.t => 'msg) =>
    Isolinear.Effect.t('msg);
};

[@deriving show]
type msg =
  | NewProvider({
      handle: int,
      id: string,
      label: string,
      rootUri: option(Oni_Core.Uri.t),
    })
  | LostProvider({handle: int})
  | NewResourceGroup({
      provider: int,
      handle: int,
      id: string,
      label: string,
    })
  | LostResourceGroup({
      provider: int,
      handle: int,
    })
  | ResourceStatesChanged({
      provider: int,
      group: int,
      spliceStart: int,
      deleteCount: int,
      additions: list(Oni_Core.SCMModels.Resource.t),
    })
  | CountChanged({
      handle: int,
      count: int,
    })
  | QuickDiffProviderChanged({
      handle: int,
      available: bool,
    })
  | CommitTemplateChanged({
      handle: int,
      template: string,
    });

let update: (msg, t) => (t, Isolinear.Effect.t(msg));

module Pane: {
  let make:
    (
      ~model: t,
      ~workingDirectory: option(string),
      ~onItemClick: Oni_Core.SCMModels.Resource.t => unit,
      ~theme: Oni_Core.Theme.t,
      ~font: Oni_Core.UiFont.t,
      unit
    ) =>
    Revery.UI.element;
};
