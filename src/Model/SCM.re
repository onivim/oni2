open Oni_Core;

module Group = {
  [@deriving show({with_path: false})]
  type t = {
    handle: int,
    id: string,
    label: string,
    hideWhenEmpty: bool,
  };
};

module Provider = {
  [@deriving show({with_path: false})]
  type t = {
    handle: int,
    id: string,
    label: string,
    rootUri: option(Uri.t),
    groups: list(Group.t),
    hasQuickDiffProvider: bool,
  };
};

[@deriving show({with_path: false})]
type t = {providers: list(Provider.t)};

let initial = {providers: []};

[@deriving show({with_path: false})]
type msg =
  | NewProvider({
      handle: int,
      id: string,
      label: string,
      rootUri: option(Uri.t),
    });
