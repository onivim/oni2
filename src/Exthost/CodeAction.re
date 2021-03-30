type t = {
  chainedCacheId: option(ChainedCacheId.t),
  title: string,
  edit: option(WorkspaceEdit.t),
  diagnostics: list(Diagnostic.t),
  command: option(Command.t),
  kind: option(string),
  isPreferred: bool,
  disabled: option(string),
};

module ProviderMetadata = {
  type t = {
    providedKinds: list(string),
    providedDocumentation: Oni_Core.StringMap.t(Command.t),
  };
};

module List = {
  type t = {
    cacheId: int,
    actions: list(t),
  };
};
