module Resource = {
  [@deriving show({with_path: false})]
  type t = {
    handle: int,
    uri: Uri.t,
    icons: list(string),
    tooltip: string,
    strikeThrough: bool,
    faded: bool,
    source: option(string),
    letter: option(string),
    color: option(string),
  };
};

module ResourceGroup = {
  [@deriving show({with_path: false})]
  type t = {
    handle: int,
    id: string,
    label: string,
    hideWhenEmpty: bool,
    resources: list(Resource.t),
  };
};

module Provider = {
  [@deriving show({with_path: false})]
  type t = {
    handle: int,
    id: string,
    label: string,
    rootUri: option(Uri.t),
    resourceGroups: list(ResourceGroup.t),
    hasQuickDiffProvider: bool,
    count: int,
    commitTemplate: string,
  };
};
